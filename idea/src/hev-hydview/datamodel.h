#ifndef HEV_HYDVIEW_DATAMODEL_H
#define HEV_HYDVIEW_DATAMODEL_H

#include "hyd_data.h"
#include <qwt_samples.h>
#include <qwt_series_data.h>
#include <QObject>
#include <QDir>
#include <vector>
#include <stdint.h>

class QStringListModel;

class HistogramSeries : public QwtSeriesData<QwtIntervalSample> {
public:
  virtual size_t size() const { return _samples.size(); }

  virtual QwtIntervalSample sample(size_t i) const {
    return QwtIntervalSample(_samples[i], _intervals[i]);
  }

  virtual QRectF boundingRect() const { return d_boundingRect; }

  HistogramSeries(std::vector<QwtInterval>& intervals,
                  std::vector<uint64_t>& samples);
  virtual ~HistogramSeries() {}

private:
  std::vector<QwtInterval> _intervals;
  std::vector<uint64_t>& _samples;
}; // class HistogramSeries

class CurveSeries : public QwtSeriesData<QPointF> {
public:
  virtual size_t size() const { return _samples.size(); }

  virtual QPointF sample(size_t i) const { return _samples[i]; }

  virtual QRectF boundingRect() const { return d_boundingRect; }

  CurveSeries(std::vector<QPointF>& samples);
  virtual ~CurveSeries() {}

private:
  std::vector<QPointF>& _samples;
}; // class CurveSeries

class DataModel : public QObject {
  Q_OBJECT

public:
  static int const NUM_BINS = 20;

  DataModel(QObject* parent = Q_NULLPTR);
  virtual ~DataModel();

  char const* shm_name() const { return _shm_name.c_str(); }

  size_t nSites() const { return _data->nSites; }
  size_t nItemsPerSite() const { return _data->nItemsPerSite; }
  size_t dimX() const { return _data->dimX; }
  size_t dimY() const { return _data->dimY; }
  size_t dimZ() const { return _data->dimZ; }
  size_t nTimesteps() const { return _data->nTimesteps; }

  float* data() { return _data->data; }

  QStringListModel* items() { return _items; }

  HistogramSeries* histogram(size_t item, size_t timestep) {
    return new HistogramSeries(_bin_widths[item], _bins[timestep][item]);
  }

  CurveSeries* curve(size_t item) {
    return new CurveSeries(_timestep_averages[item]);
  }

  enum Operation {
    Plus,
    Subtract,
    Multiply,
    Divide,
  };

public slots:
  void load(QString const& dirName);
  void insert(int column1Index, int column2Index, Operation operation);

signals:
  void loaded(QString const&, QString const&);
  void inserted(QString const&);

private:
  static float doOperation(Operation op, float a, float b);

  QStringListModel* _items;

  std::string const _shm_name;
#ifndef _WIN32
  int _data_shm;
#endif
  hyd_data* _data;

  std::vector<QwtInterval> _intervals;                     // per item
  std::vector<std::vector<QwtInterval> > _bin_widths;      // per item
  std::vector<std::vector<std::vector<uint64_t> > > _bins; // timestep, item
  std::vector<std::vector<QPointF> > _timestep_averages;   // per item
}; // class DataModel

Q_DECLARE_METATYPE(DataModel::Operation);

#endif // HEV_HYDVIEW_DATAMODEL_H

