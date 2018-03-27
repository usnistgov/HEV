#include "datamodel.h"
#include <QStringListModel>
#include <QtWidgets>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
// clang-format off
#ifdef WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <io.h>
#  undef WIN32_LEAN_AND_MEAN
#  undef NOMINMAX
#else
#  include <sys/mman.h>
#  include <unistd.h>
#endif // WIN32
// clang-format on

HistogramSeries::HistogramSeries(std::vector<QwtInterval>& intervals,
                                 std::vector<uint64_t>& samples)
  : _intervals(intervals), _samples(samples) {
  d_boundingRect = QRectF(0, 0, intervals.back().maxValue(),
                          *std::max_element(samples.begin(), samples.end()));
} // HistogramSeries::HistogramSeries

CurveSeries::CurveSeries(std::vector<QPointF>& samples) : _samples(samples) {
  qreal max_height = samples[0].y();
  for (size_t i = 1; i < samples.size(); ++i) {
    max_height = std::max(max_height, samples[i].y());
  }
  d_boundingRect = QRectF(0, 0, samples.size() - 1, max_height);
} // CurveSeries::CurveSeries

DataModel::DataModel(QObject* parent)
  : QObject(parent)
  , _items(new QStringListModel(this))
#ifndef _WIN32
  , _shm_name(std::string("/hydviewSharedMem_") + getenv("USER"))
  , _data_shm(-1)
#endif
{
  qRegisterMetaType<Operation>("DataModel::Operation");
} // DataModel::DataModel

DataModel::~DataModel() {
#ifdef _WIN32
  free(_data);
#else
  munmap(_data, sizeof(hyd_data) + _data->dataSize);
  close(_data_shm);
#endif
} // DataModel::~DataModel

struct bimgHeader {
  uint8_t magic[4];
  uint8_t unused[4];
  double time;
  uint64_t nItemsPerSite;
  uint64_t dims[3];
  uint8_t unused2[2048];
}; // struct bimgHeader

static size_t const NAME_LEN = 128;
static char const* const MAGIC = "HYIB";

static QStringList readHeader(QString const& fileName, bimgHeader* header) {
  QStringList columns;
  char name[NAME_LEN];

  if (fileName.isEmpty()) return columns;
  ::memset(header, 0, sizeof(*header));

#ifdef WIN32
  int fd = _open(fileName.toLocal8Bit().constData(), _O_RDONLY | _O_BINARY);
#else
  int fd = ::open(fileName.toLocal8Bit().constData(), O_RDONLY);
#endif // WIN32
  if (fd < 0) return columns;

#ifndef WIN32
  ::posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
  ::posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);
  ::posix_fadvise(fd, 0, 0, POSIX_FADV_WILLNEED);
#endif // WIN32

  errno = EIO;
#ifdef WIN32
  if (_read(fd, header, sizeof(*header)) != sizeof(*header)) goto err;
#else
  if (::read(fd, header, sizeof(*header)) != sizeof(*header)) goto err;
#endif // WIN32

  errno = EINVAL;
  if (::strcmp(reinterpret_cast<char const*>(header->magic), MAGIC) != 0) {
    goto err;
  }

  for (uint64_t i = 0; i < header->nItemsPerSite; ++i) {
    errno = EIO;
#ifdef WIN32
    if (_read(fd, name, NAME_LEN) != NAME_LEN) goto err;
#else
    if (::read(fd, name, NAME_LEN) != NAME_LEN) goto err;
#endif // WIN32
    columns << name;
  }

err:
#ifdef WIN32
  if (fd >= 0) _close(fd);
#else
  if (fd >= 0) ::close(fd);
#endif // WIN32
  return columns;
} // readHeader

static bool readData(QString const& fileName, double* dst) {
  bool ret = false;
  if (fileName.isEmpty()) return ret;

  bimgHeader header;
  ::memset(&header, 0, sizeof(header));

#ifdef WIN32
  int fd = _open(fileName.toLocal8Bit().constData(), _O_RDONLY | _O_BINARY);
#else
  int fd = ::open(fileName.toLocal8Bit().constData(), O_RDONLY);
#endif // WIN32
  if (fd < 0) return ret;

#ifndef WIN32
  ::posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
  ::posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);
  ::posix_fadvise(fd, 0, 0, POSIX_FADV_WILLNEED);
#endif // WIN32

  errno = EIO;
#ifdef WIN32
  if (_read(fd, &header, sizeof(header)) != sizeof(header)) goto err;
#else
  if (::read(fd, &header, sizeof(header)) != sizeof(header)) goto err;
#endif // WIN32

  errno = EINVAL;
  if (::strcmp(reinterpret_cast<char const*>(header.magic), MAGIC) != 0) {
    goto err;
  }

  { // scope for const variables below
    size_t const count =
      header.dims[0] * header.dims[1] * header.dims[2] * header.nItemsPerSite;
    size_t const dataSize = count * sizeof(double);
    errno = EIO;

#ifdef WIN32
    if (_lseek(fd, static_cast<long>(NAME_LEN * header.nItemsPerSite),
               SEEK_CUR) < 0) {
      goto err;
    }
    int const nread = _read(fd, dst, static_cast<unsigned int>(dataSize));
    if (static_cast<size_t>(nread) != dataSize) {
      if (nread != -1)
        qWarning() << "short read: " << nread << " (" << errno << ")";
      goto err;
    }
#else
    if (::lseek(fd, NAME_LEN * header.nItemsPerSite, SEEK_CUR) < 0) goto err;
    ssize_t const nread = ::read(fd, dst, dataSize);
    if (static_cast<size_t>(nread) != dataSize) {
      if (nread != -1)
        qWarning() << "short read: " << nread << " (" << errno << ")";
      goto err;
    }
#endif // WIN32
  }

  ret = true;
err:
  if (!ret) {
    qWarning(QString("Unable to read %1: %2")
               .arg(fileName)
               .arg(strerror(errno))
               .toLocal8Bit()
               .constData());
  }

#ifdef WIN32
  if (fd >= 0) _close(fd);
#else
  if (fd >= 0) ::close(fd);
#endif // WIN32
  return ret;
} // readData

static QStringList readXml(QString fn, QString key, QString subkey) {
  QStringList sl;

  QFile f(fn);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return sl;

  QXmlStreamReader xml(&f);
  bool in = false;

  while (!xml.atEnd()) {
    switch (xml.readNext()) {
    case QXmlStreamReader::StartElement:
      if (xml.name() == key) in = true;
      if (in && xml.name() == subkey) sl << xml.readElementText();
      break;
    case QXmlStreamReader::EndElement:
      if (xml.name() == key) in = false;
      break;
    default: break;
    }
  }

  return sl;
} // readXml

void DataModel::load(QString const& dirName) {
  QDir dir(dirName);

  // get the list of timestamp files
  QStringList bimgsFNs = dir.entryList(
    QStringList("*.bimg"), QDir::Files | QDir::Readable, QDir::Name);
  if (bimgsFNs.empty()) {
    emit loaded(dir.path(),
                tr("No files with `.bimg` extension in %1").arg(dir.path()));
    return;
  }

  // read the header of the first file
  bimgHeader header;
  QStringList columns = readHeader(
    dir.absoluteFilePath(bimgsFNs.first()).toLocal8Bit().constData(), &header);
  if (columns.isEmpty()) {
    emit loaded(
      dir.path(),
      tr("Error reading %1").arg(dir.absoluteFilePath(bimgsFNs.first())));
    return;
  }

  // replace the "Reaction X" name with the actual name of the reaction
  if (dir.exists("reactions")) {
    dir.cd("reactions");
    QStringList reactionsFNs = dir.entryList(
      QStringList("*.xml"), QDir::Files | QDir::Readable, QDir::Name);

    if (!reactionsFNs.empty()) {
      QString const reactionsFN = dir.absoluteFilePath(reactionsFNs.first());
      dir.cdUp();

      QStringList const reactions = readXml(reactionsFN, "reaction", "rxname");
      if (!reactions.empty()) {
        QStringList::const_iterator reactionsIterator = reactions.begin();

        for (int i = 0; i < columns.size(); ++i) {
          if (columns[i].startsWith("Reaction")) {
            columns[i] = *reactionsIterator;
            reactionsIterator++;
          }
        }
      }
    }
  }

  size_t const dataSize = header.dims[0] * header.dims[1] * header.dims[2] *
                          header.nItemsPerSite *
                          static_cast<size_t>(bimgsFNs.size()) * sizeof(float);

#ifdef _WIN32
  _data = reinterpret_cast<hyd_data*>(malloc(dataSize));
#else
  // open the shared memory segment
  _data_shm = shm_open(_shm_name.c_str(), O_RDWR | O_CREAT | O_TRUNC,
                       S_IRWXU | S_IRWXG | S_IRWXO);
  if (_data_shm < 0) {
    emit loaded(dir.path(),
                tr("Error opening shared memory: %1").arg(strerror(errno)));
    return;
  }

  // size the shared memory segment
  if (ftruncate(_data_shm, sizeof(hyd_data) + dataSize) == -1) {
    emit loaded(dir.path(),
                tr("Error sizing shared memory: %1").arg(strerror(errno)));
    return;
  }

  // map the shared memory segment
  _data = reinterpret_cast<hyd_data*>(mmap(NULL, sizeof(hyd_data) + dataSize,
                                           PROT_READ | PROT_WRITE, MAP_SHARED,
                                           _data_shm, 0));
  if (_data == MAP_FAILED) {
    emit loaded(dir.path(),
                tr("Error mapping shared memory: %1").arg(strerror(errno)));
    return;
  }
#endif

  // set data members
  _items->setStringList(columns);
  _data->dataSize = dataSize;
  _data->nSites = header.dims[0] * header.dims[1] * header.dims[2];
  _data->nItemsPerSite = header.nItemsPerSite;
  _data->dimX = header.dims[0];
  _data->dimY = header.dims[1];
  _data->dimZ = header.dims[2];
  _data->nTimesteps = static_cast<size_t>(bimgsFNs.size());
  _data->nAddedItems = 0;

  // local const variables for thread-safety
  int const nSites = static_cast<int>(_data->nSites);
  int const nItemsPerSite = static_cast<int>(_data->nItemsPerSite);
  int const dimX = static_cast<int>(_data->dimX);
  int const dimY = static_cast<int>(_data->dimY);
  int const dimZ = static_cast<int>(_data->dimZ);
  int const nTimesteps = static_cast<int>(_data->nTimesteps);

  // read all the files
  bool abort = false;

  #pragma omp parallel for default(none) shared(dir, bimgsFNs, abort)
  for (int i = 0; i < nTimesteps; ++i) {
    QString const fn = dir.absoluteFilePath(bimgsFNs[i]);
    float* dst = _data->data + i * nItemsPerSite * nSites;
    std::vector<double> scratch(static_cast<size_t>(nItemsPerSite * nSites));

    #pragma omp flush(abort)
    if (!abort) {
      bool ok = readData(fn, scratch.data());
      if (!ok) {
        abort = true;
        #pragma omp flush(abort)
      }

      // rearrange data for this timestep into item, z, y, z
      size_t pos = 0;
      for (int z = 0; z < dimZ; ++z) {
        for (int y = 0; y < dimY; ++y) {
          for (int x = 0; x < dimX; ++x) {
            for (int n = 0; n < nItemsPerSite; ++n, ++pos) {
              int const doffset =
                n * dimZ * dimY * dimX + z * dimY * dimX + y * dimX + x;
              dst[doffset] = static_cast<float>(scratch[pos]);
            }
          }
        }
      }
    }
  }

  if (abort) {
    emit loaded(dir.path(), tr("Error reading data files"));
    return;
  }

  // initialize the other data structures
  _intervals.resize(static_cast<size_t>(nItemsPerSite));
  _bin_widths.resize(static_cast<size_t>(nItemsPerSite));

  for (size_t i = 0; i < static_cast<size_t>(nItemsPerSite); ++i) {
    _intervals[i].setInterval(INFINITY, -INFINITY);
    _bin_widths[i].resize(NUM_BINS);
  }

  _bins.resize(static_cast<size_t>(nTimesteps));
  for (size_t i = 0; i < static_cast<size_t>(nTimesteps); ++i) {
    _bins[i].resize(static_cast<size_t>(nItemsPerSite));
    for (size_t j = 0; j < static_cast<size_t>(nItemsPerSite); ++j) {
      _bins[i][j].resize(NUM_BINS);
    }
  }

  _timestep_averages.resize(static_cast<size_t>(nItemsPerSite));
  for (size_t i = 0; i < static_cast<size_t>(nItemsPerSite); ++i) {
    _timestep_averages[i].resize(static_cast<size_t>(nTimesteps));
  }

  // compute the intervals for each item across all timesteps
  for (int i = 0; i < nTimesteps; ++i) {
    int const il = i;

    #pragma omp parallel for default(none)
    for (int j = 0; j < nItemsPerSite; ++j) {
      QwtInterval* interval = _intervals.data() + j;
      float const* src = _data->data + il * nItemsPerSite * nSites + j * nSites;

      for (int k = 0; k < nSites; ++k, ++src) {
        interval->setInterval(
          std::min(interval->minValue(), static_cast<double>(*src)),
          std::max(interval->maxValue(), static_cast<double>(*src)));
      }
    }
  }

  // compute the bin widths for each item
  #pragma omp parallel for default(none)
  for (int i = 0; i < nItemsPerSite; ++i) {
    QwtInterval* interval = _intervals.data() + i;
    std::vector<QwtInterval>* bin_width = _bin_widths.data() + i;
    double const width = interval->width() / static_cast<double>(NUM_BINS);

    for (size_t j = 0; j < NUM_BINS; ++j) {
      (*bin_width)[j].setInterval(width * j, width * (j + 1),
                                  QwtInterval::ExcludeMaximum);
    }

    (*bin_width).front().setMinValue(interval->minValue());

    // make the max of the final bin just a bit larger than the true max value
    // this ensures the histogram will include the max value since we are
    // excluding the maximum on the QwtInterval so as to not double count
    (*bin_width).back().setMaxValue(interval->maxValue() + 1e-10);
  }

  // compute the histogram bins and averages for each item per timestep
  #pragma omp parallel for default(none)
  for (int i = 0; i < nTimesteps; ++i) {
    std::vector<std::vector<uint64_t> >* bin = _bins.data() + i;

    for (int j = 0; j < nItemsPerSite; ++j) {
      std::vector<QwtInterval>* bin_width = _bin_widths.data() + j;
      float const* src = _data->data + i * nItemsPerSite * nSites + j * nSites;

      for (int k = 0; k < nSites; ++k, ++src) {
        for (int l = 0; l < NUM_BINS; ++l) {
          if ((*bin_width)[l].contains(static_cast<double>(*src))) {
            (*bin)[j][l] += 1;
            break;
          }
        }

        _timestep_averages[j][i].setX(i);
        _timestep_averages[j][i].setY(_timestep_averages[j][i].y() +
                                      static_cast<double>(*src));
      }
    }
  }

  // finish average computation
  double const dnSites = static_cast<double>(nSites);
  #pragma omp parallel for default(none)
  for (int i = 0; i < nItemsPerSite; ++i) {
    std::vector<QPointF>* timestep_average = _timestep_averages.data() + i;

    for (size_t j = 0; j < static_cast<size_t>(nTimesteps); ++j) {
      (*timestep_average)[j].setY((*timestep_average)[j].y() / dnSites);
    }
  }

  // all done
  emit loaded(dir.path(), "");
} // DataModel::load

void DataModel::insert(int column1Index, int column2Index,
                       Operation operation) {
  int const nSites = static_cast<int>(_data->nSites);
  int const nItemsPerSite = static_cast<int>(_data->nItemsPerSite);
  int const nTimesteps = static_cast<int>(_data->nTimesteps);

  size_t dataSize = _data->dataSize;

#ifndef _WIN32
  // unmap the shared memory segment
  if (munmap(_data, sizeof(hyd_data) + dataSize) == -1) {
    emit inserted(tr("Error unmapping shared memory: %1").arg(strerror(errno)));
    return;
  }
#endif

  dataSize += nTimesteps * nSites * sizeof(float);

#ifdef _WIN32
  _data = reinterpret_cast<hyd_data*>(realloc(_data, dataSize));
  if (_data == NULL) {
    emit inserted(tr("Error resizing data: %1").arg(strerror(errno)));
  }
#else
  // resize the shared memory segment
  if (ftruncate(_data_shm, sizeof(hyd_data) + dataSize) == -1) {
    emit inserted(tr("Error resizing shared memory: %1").arg(strerror(errno)));
    return;
  }

  // remap the shared memory segment
  _data = reinterpret_cast<hyd_data*>(mmap(NULL, sizeof(hyd_data) + dataSize,
                                           PROT_READ | PROT_WRITE, MAP_SHARED,
                                           _data_shm, 0));
  if (_data == MAP_FAILED) {
    emit inserted(tr("Error remapping shared memory: %1").arg(strerror(errno)));
    return;
  }
#endif

  _data->dataSize = dataSize;
  _data->nAddedItems += 1;

  int const dstIndex = _data->nAddedItems - 1;
  Operation const ccop = operation;
  int const c1index = column1Index;
  int const c2index = column2Index;

  // compute the new column data
  #pragma omp parallel for default(none)
  for (int i = 0; i < nTimesteps; ++i) {
    float* dst = _data->data + (nTimesteps - 1) * nItemsPerSite * nSites +
                 dstIndex * nTimesteps * nSites + i * nSites;
    float const* c1data =
      _data->data + i * nItemsPerSite * nSites + c1index * nSites;
    float const* c2data =
      _data->data + i * nItemsPerSite * nSites + c2index * nSites;

    for (int k = 0; k < nSites; ++k, ++c1data, ++c2data, ++dst) {
      *dst = DataModel::doOperation(ccop, *c1data, *c2data);
    }
  }

  // resize the data structures
  _intervals.push_back(QwtInterval(INFINITY, -INFINITY));
  _bin_widths.push_back(std::vector<QwtInterval>(NUM_BINS));

  for (size_t i = 0; i < static_cast<size_t>(nTimesteps); ++i) {
    _bins[i].push_back(std::vector<uint64_t>(NUM_BINS));
  }

  _timestep_averages.push_back(
    std::vector<QPointF>(static_cast<size_t>(nTimesteps)));

  // compute the interval for the new column across all timesteps
  QwtInterval* interval = &_intervals.back();
  for (int i = 0; i < nTimesteps; ++i) {
    float const* src = _data->data + (nTimesteps - 1) * nItemsPerSite * nSites +
                       dstIndex * nTimesteps * nSites + i * nSites;

    for (int k = 0; k < nSites; ++k, ++src) {
      interval->setInterval(
        std::min(interval->minValue(), static_cast<double>(*src)),
        std::max(interval->maxValue(), static_cast<double>(*src)));
    }
  }

  // compute the bin widths for the new column
  std::vector<QwtInterval>* bin_width = &_bin_widths.back();
  double const width = interval->width() / static_cast<double>(NUM_BINS);

  for (size_t j = 0; j < NUM_BINS; ++j) {
    (*bin_width)[j].setInterval(width * j, width * (j + 1),
                                QwtInterval::ExcludeMaximum);
  }

  (*bin_width).front().setMinValue(interval->minValue());

  // make the max of the final bin just a bit larger than the true max value
  // this ensures the histogram will include the max value since we are
  // excluding the maximum on the QwtInterval so as to not double count
  (*bin_width).back().setMaxValue(interval->maxValue() + 1e-10);

  // compute the histogram bins and averages for the new column per timestep
  for (size_t i = 0; i < static_cast<size_t>(nTimesteps); ++i) {
    std::vector<uint64_t>* bin = &_bins[i].back();
    float const* src = _data->data + (nTimesteps - 1) * nItemsPerSite * nSites +
                       dstIndex * nTimesteps * nSites + i * nSites;

    for (int k = 0; k < nSites; ++k, ++src) {
      for (size_t l = 0; l < NUM_BINS; ++l) {
        if ((*bin_width)[l].contains(static_cast<double>(*src))) {
          (*bin)[l] += 1;
          break;
        }
      }

      _timestep_averages.back()[i].setX(i);
      _timestep_averages.back()[i].setY(_timestep_averages.back()[i].y() +
                                        static_cast<double>(*src));
    }
  }

  // finish average computation
  double const dnSites = static_cast<double>(nSites);
  std::vector<QPointF>* timestep_average = &_timestep_averages.back();
  for (size_t j = 0; j < static_cast<size_t>(nTimesteps); ++j) {
    (*timestep_average)[j].setY((*timestep_average)[j].y() / dnSites);
  }

  emit inserted("");
} // DataModel::insert

float DataModel::doOperation(Operation op, float a, float b) {
  switch (op) {
  case Plus: return a + b;
  case Subtract: return a - b;
  case Multiply: return a * b;
  case Divide: return a / b;
  }
#ifdef _WIN32
  __assume(0);
#else
  __builtin_unreachable();
#endif
}

