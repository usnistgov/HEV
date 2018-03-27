#include "renderwidget.h"
#include "datamodel.h"
#include "tables.h"
#include <QFile>
#include <QOpenGLDebugMessage>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLVertexArrayObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QtDebug>

// see kernels/isosurface.cl
#define NTHREADS 32
#define MAX_VERTS_PER_CELL 15

static char const* clstrerror(cl_int ec) {
  switch (ec) {
  case CL_SUCCESS: return "Success";
  case CL_DEVICE_NOT_FOUND: return "Device not found";
  case CL_DEVICE_NOT_AVAILABLE: return "Device not available";
  case CL_COMPILER_NOT_AVAILABLE: return "Compiler not available";
  case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "Mem object allocation failure";
  case CL_OUT_OF_RESOURCES: return "Out of resources";
  case CL_OUT_OF_HOST_MEMORY: return "Out of host memory";
  case CL_PROFILING_INFO_NOT_AVAILABLE: return "Profiling info not available";
  case CL_MEM_COPY_OVERLAP: return "Mem copy overlap";
  case CL_IMAGE_FORMAT_MISMATCH: return "Image format mismatch";
  case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "Image format not supported";
  case CL_BUILD_PROGRAM_FAILURE: return "Build program failured";
  case CL_MAP_FAILURE: return "Map failure";
  case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "Misaligned sub buffer offset";
  case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
    return "Exec status error for events in wait list";
  case CL_INVALID_VALUE: return "Invalid value";
  case CL_INVALID_DEVICE_TYPE: return "Invalid device type";
  case CL_INVALID_PLATFORM: return "Invalid platform";
  case CL_INVALID_DEVICE: return "Invalid device";
  case CL_INVALID_CONTEXT: return "Invalid context";
  case CL_INVALID_QUEUE_PROPERTIES: return "Invalid queue properties";
  case CL_INVALID_COMMAND_QUEUE: return "Invalid command queue";
  case CL_INVALID_HOST_PTR: return "Invalid host ptr";
  case CL_INVALID_MEM_OBJECT: return "Invalid mem object";
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
    return "Invalid image format descriptor";
  case CL_INVALID_IMAGE_SIZE: return "Invalid image size";
  case CL_INVALID_SAMPLER: return "Invalid sampler";
  case CL_INVALID_BINARY: return "Invalid binary";
  case CL_INVALID_BUILD_OPTIONS: return "Invalid build options";
  case CL_INVALID_PROGRAM: return "Invalid program";
  case CL_INVALID_PROGRAM_EXECUTABLE: return "Invalid program executable";
  case CL_INVALID_KERNEL_NAME: return "Invalid kernel name";
  case CL_INVALID_KERNEL_DEFINITION: return "Invalid kernel definition";
  case CL_INVALID_KERNEL: return "Invalid kernel";
  case CL_INVALID_ARG_INDEX: return "Invalid arg index";
  case CL_INVALID_ARG_VALUE: return "Invalid arg value";
  case CL_INVALID_ARG_SIZE: return "Invalid arg size";
  case CL_INVALID_KERNEL_ARGS: return "Invalid kernel args";
  case CL_INVALID_WORK_DIMENSION: return "Invalid work dimension";
  case CL_INVALID_WORK_GROUP_SIZE: return "Invalid work group size";
  case CL_INVALID_WORK_ITEM_SIZE: return "Invalid work item size";
  case CL_INVALID_GLOBAL_OFFSET: return "Invalid global offset";
  case CL_INVALID_EVENT_WAIT_LIST: return "Invalid event wait list";
  case CL_INVALID_EVENT: return "Invalid event";
  case CL_INVALID_OPERATION: return "Invalid operation";
  case CL_INVALID_GL_OBJECT: return "Invalid GL object";
  case CL_INVALID_BUFFER_SIZE: return "Invalid buffer size";
  case CL_INVALID_MIP_LEVEL: return "Invalid mip level";
  case CL_INVALID_GLOBAL_WORK_SIZE: return "Invalid global work size";
  case CL_INVALID_PROPERTY: return "Invalid property";
  }
  return "Unknown error";
} // clstrerror

RenderWidget::RenderWidget(QWidget* parent)
  : QOpenGLWidget(parent)
  , _timestep(0)
  , _ctrlDown(false)
  , _mouseDrag(false)
  , _triTable(0)
  , _numVertsTable(0)
  , _logger(Q_NULLPTR)
  , _gl_program(Q_NULLPTR)
  , _plane(Q_NULLPTR)
  , _uniformLocations(NumUniforms) {
  setFocusPolicy(Qt::ClickFocus);

  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setSamples(16);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setOption(QSurfaceFormat::DebugContext);
  QSurfaceFormat::setDefaultFormat(format);
  setFormat(format);

  _camera.translate(0, 0, 3);

  //
  // Initialize OpenCL
  //
  cl_int err = clGetPlatformIDs(1, &_platform, Q_NULLPTR);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot get OpenCL platform: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }

  err = clGetDeviceIDs(_platform, CL_DEVICE_TYPE_GPU, 1, &_device, Q_NULLPTR);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot get OpenCL devices: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }

  _context =
    clCreateContext(Q_NULLPTR, 1, &_device, Q_NULLPTR, Q_NULLPTR, &err);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot create OpenCL context: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }

  cl_command_queue_properties props = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;

  _queue = clCreateCommandQueue(_context, _device, props, &err);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot create OpenCL command queue: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }
} // RenderWidget::RenderWidget

RenderWidget::~RenderWidget() {
  makeCurrent();

  if (_plane) delete _plane;
  for (size_t i = 0; i < _surfaces.size(); ++i) {
    for (size_t j = 0; j < _surfaces[i].size(); ++j) { delete _surfaces[i][j]; }
  }
  delete _gl_program;

  clReleaseMemObject(_numVertsTable);
  clReleaseMemObject(_triTable);

  clReleaseKernel(_iso_kernel);
  clReleaseKernel(_grad_kernel);
  clReleaseProgram(_cl_program);
  clReleaseCommandQueue(_queue);
  clReleaseContext(_context);
} // RenderWidget::~RenderWidget

void RenderWidget::setModel(DataModel* model) {
  _model = model;
  if (_surfaces.empty()) _surfaces.resize(_model->nTimesteps());

  makeCurrent();
  std::vector<QVector3D> vbo;

  vbo.push_back(QVector3D(_model->dimY(), 0, 0));
  vbo.push_back(QVector3D(0, 0, 0));
  vbo.push_back(QVector3D(0, _model->dimX(), 0));
  vbo.push_back(QVector3D(0, 0, 0));
  vbo.push_back(QVector3D(0, 0, 0));
  vbo.push_back(QVector3D(0, 0, 0));

  vbo.push_back(QVector3D(_model->dimY(), 0, 0));
  vbo.push_back(QVector3D(0, 0, 0));
  vbo.push_back(QVector3D(_model->dimY(), _model->dimX(), 0));
  vbo.push_back(QVector3D(0, 0, 0));
  vbo.push_back(QVector3D(0, _model->dimX(), 0));
  vbo.push_back(QVector3D(0, 0, 0));

  _plane = new Drawable(vbo, vbo.size() / 2, Qt::gray, 1.f);
  _plane->mode = GL_LINE;
  doneCurrent();
} // RenderWidget::setModel

void RenderWidget::timestepChanged(int timestep) {
  _timestep = timestep;
} // RenderWidget::timestepChanged

void RenderWidget::generate(int /*timestep*/, int column, float isoValue,
                            QColor const& color) {
  int const nTimesteps = static_cast<int>(_model->nTimesteps());
  int const nSites = static_cast<int>(_model->nSites());
  int const nItemsPerSite = static_cast<int>(_model->nItemsPerSite());
  int const dimX = static_cast<int>(_model->dimX());
  int const dimY = static_cast<int>(_model->dimY());
  int const dimZ = static_cast<int>(_model->dimZ());
  int const cind = column;
  float const iso = isoValue;

  float* data = _model->data();
  std::vector<std::vector<QVector3D>> vbos(nTimesteps);

  bool abort = false;

  #pragma omp parallel for default(none) shared(data, vbos, abort)
  for (int i = 0; i < nTimesteps; ++i) {
    std::vector<QVector3D>& vbo = vbos[i];

    float* src;
    if (cind < nItemsPerSite) {
      src = data + (i * nItemsPerSite * nSites) + (cind * nSites);
    } else {
      src = data + (nTimesteps - 1) * nItemsPerSite * nSites +
            (cind - nItemsPerSite) * nTimesteps * nSites + i * nSites;
    }

    #pragma omp flush(abort)
    if (!abort) {
      bool ok =
        genSurface(src, i, cind, iso, dimX, dimY, dimZ, vbo);
      if (!ok) {
        abort = true;
        #pragma omp flush(abort)
      }
    }
  }

  if (abort) {
    emit generateDone(tr("error generating isosurface"));
    return;
  }

  float const scale =
    1.f / static_cast<float>(std::max(std::max(dimX, dimY), dimZ));

  makeCurrent();

  for (int i = 0; i < nTimesteps; ++i) {
    _gl_program->bind();
    _surfaces[i].push_back(
      new Drawable(vbos[i], vbos[i].size() / 2, color, scale));

    _gl_program->enableAttributeArray(0);
    _gl_program->enableAttributeArray(1);
    _gl_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 2 * sizeof(QVector3D));
    _gl_program->setAttributeBuffer(1, GL_FLOAT, sizeof(QVector3D), 3,
                                    2 * sizeof(QVector3D));

    _surfaces[i].back()->vao->release();
    _surfaces[i].back()->vbo->release();
    _gl_program->release();
  }

  doneCurrent();
  emit generateDone("");
} // RenderWidget::generate

void RenderWidget::remove(int row) {
  makeCurrent();
  for (size_t i = 0; i < _model->nTimesteps(); ++i) {
    delete _surfaces[i][row];
    _surfaces[i].erase(_surfaces[i].begin() + row);
  }
  doneCurrent();
} // RenderWidget::remove

void RenderWidget::setVisible(int row, bool value) {
  for (size_t i = 0; i < _model->nTimesteps(); ++i) {
    _surfaces[i][row]->visible = value;
  }
} // RenderWidget::setVisible

void RenderWidget::clipPlaneChanged(float x, float y, float z, float h, float p,
                                    float r) {
  QQuaternion qh(qDegreesToRadians(h), 0, 0, 1);
  QQuaternion qp(qDegreesToRadians(p), 1, 0, 0);
  QQuaternion qr(qDegreesToRadians(r), 0, 1, 0);

  QMatrix4x4 mat;
  mat.rotate(qr * qp * qh);
  mat.translate(x, y, z);

  _plane->testMat = mat;
} // RenderWidget::clipPlaneChanged

void RenderWidget::initializeGL() {
  initializeOpenGLFunctions();
  connect(this, &QOpenGLWidget::frameSwapped, this, &RenderWidget::update);

  QString const glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
  QString const glVersion =
    reinterpret_cast<const char*>(glGetString(GL_VERSION));

  QString glProfile;
  switch (format().profile()) {
  case QSurfaceFormat::NoProfile: glProfile = "(None)"; break;
  case QSurfaceFormat::CoreProfile: glProfile = "(Core)"; break;
  case QSurfaceFormat::CompatibilityProfile:
    glProfile = "(Compatibility)";
    break;
  }

  qInfo() << qPrintable(glType) << qPrintable(glVersion)
          << qPrintable(glProfile);

  _logger = new QOpenGLDebugLogger(this);
  if (_logger->initialize()) {
    connect(_logger, &QOpenGLDebugLogger::messageLogged, this,
            &RenderWidget::messageLogged);
    _logger->startLogging();
  }

  glClearColor(0.f, 0.f, 0.f, 1.f);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  QFile vertex_source_file(":/shaders/hydview.vert");
  vertex_source_file.open(QIODevice::ReadOnly);

  QFile fragment_source_file(":/shaders/hydview.frag");
  fragment_source_file.open(QIODevice::ReadOnly);

  _gl_program = new QOpenGLShaderProgram(this);
  _gl_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                       vertex_source_file.readAll());
  _gl_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       fragment_source_file.readAll());
  _gl_program->link();
  _gl_program->bind();

  _uniformLocations[ProjectionFromCamera] =
    _gl_program->uniformLocation("projectionFromCamera");
  _uniformLocations[CameraFromWorld] =
    _gl_program->uniformLocation("cameraFromWorld");
  _uniformLocations[WorldFromModel] =
    _gl_program->uniformLocation("worldFromModel");
  _uniformLocations[NormalMatrix] =
    _gl_program->uniformLocation("normalMatrix");
  _uniformLocations[ClipPlane] = _gl_program->uniformLocation("clipPlane");
  _uniformLocations[CameraPosition] =
    _gl_program->uniformLocation("cameraPosition");
  _uniformLocations[MColor] = _gl_program->uniformLocation("mcolor");
  _uniformLocations[Refindex] = _gl_program->uniformLocation("refindex");
  _uniformLocations[Shininess] = _gl_program->uniformLocation("shininess");

  _gl_program->release();

  initializeCL();
} // RenderWiget::initializeGL

void RenderWidget::initializeCL() {
  QFile kernel_source_file(":/kernels/isosurface.cl");
  kernel_source_file.open(QIODevice::ReadOnly);
  QByteArray const kernel_source = kernel_source_file.readAll();
  const char* src = kernel_source.constData();
  size_t len = static_cast<size_t>(kernel_source.size());

  cl_int err;
  _cl_program = clCreateProgramWithSource(_context, 1, &src, &len, &err);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot create isosurface program: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }

  err =
    clBuildProgram(_cl_program, 0, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
  if (err != CL_SUCCESS) {
    size_t logSize;
    clGetProgramBuildInfo(_cl_program, _device, CL_PROGRAM_BUILD_LOG, 0,
                          Q_NULLPTR, &logSize);
    QByteArray log(static_cast<int>(logSize + 1), '\0');

    clGetProgramBuildInfo(_cl_program, _device, CL_PROGRAM_BUILD_LOG,
                          logSize + 1, log.data(), Q_NULLPTR);
    qFatal(tr("cannot build isosurface program: %1\n%2")
             .arg(clstrerror(err))
             .arg(log.constData())
             .toLocal8Bit()
             .constData());
  }

  _iso_kernel = clCreateKernel(_cl_program, "isosurface", &err);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot create isosurface kernel: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }

  _grad_kernel = clCreateKernel(_cl_program, "gradient", &err);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot create gradient kernel: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }

  cl_image_format imageFormat = {CL_R, CL_UNSIGNED_INT8};

  _triTable = clCreateImage2D(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                              &imageFormat, 16, 256, 0, triTable, &err);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot create triangle lookup table: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }

  _numVertsTable =
    clCreateImage2D(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    &imageFormat, 256, 1, 0, numVertsTable, &err);
  if (err != CL_SUCCESS) {
    qFatal(tr("cannot create num verts lookup table: %1")
             .arg(clstrerror(err))
             .toLocal8Bit()
             .constData());
  }
} // RenderWidget::initializeCL

void RenderWidget::resizeGL(int w, int h) {
  _projection.setToIdentity();
  _projection.perspective(45.f, w / static_cast<float>(h), .01f, 1000.f);
  glViewport(0, 0, w, h);
} // RenderWidget::resizeGL

void RenderWidget::paintGL() {
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (!_plane) return;

  _gl_program->bind();
  _gl_program->setUniformValue(_uniformLocations[ProjectionFromCamera],
                               _projection);
  _gl_program->setUniformValue(_uniformLocations[CameraFromWorld],
                               _camera.toMatrix());
  _gl_program->setUniformValue(_uniformLocations[CameraPosition],
                               _camera.translation());

  //_gl_program->setUniformValue(_uniformLocations[ClipPlane],
  //                             QVector4D(0, -1, 0, 0));
  // glEnable(GL_CLIP_PLANE0);

  size_t const num = _surfaces.empty() ? 0 : _surfaces[_timestep].size();
  for (size_t i = 0; i < num; ++i) {
    Drawable* surface = _surfaces[_timestep][i];
    if (!surface->visible) continue;

    surface->vao->bind();
    glPolygonMode(GL_FRONT_AND_BACK, surface->mode);

    _gl_program->setUniformValue(_uniformLocations[WorldFromModel],
                                 _root.toMatrix() *
                                   surface->worldFromModel.toMatrix());
    _gl_program->setUniformValue(
      _uniformLocations[NormalMatrix],
      (_root.toMatrix() * surface->worldFromModel.toMatrix()).normalMatrix());

    _gl_program->setUniformValue(_uniformLocations[MColor], surface->color);
    _gl_program->setUniformValue(_uniformLocations[Refindex], 0.4f);
    _gl_program->setUniformValue(_uniformLocations[Shininess], 1.f);

    glDrawArrays(GL_TRIANGLES, 0, surface->ntris);
    surface->vao->release();
  }

  glDisable(GL_DEPTH_TEST);

  _plane->vao->bind();
  glPolygonMode(GL_FRONT_AND_BACK, _plane->mode);

  _gl_program->setUniformValue(_uniformLocations[WorldFromModel],
                               _plane->testMat);
                               //_plane->worldFromModel.toMatrix());
  _gl_program->setUniformValue(
    _uniformLocations[NormalMatrix],
    _plane->testMat.normalMatrix());
    //_plane->worldFromModel.toMatrix().normalMatrix());

  _gl_program->setUniformValue(_uniformLocations[MColor], _plane->color);
  _gl_program->setUniformValue(_uniformLocations[Refindex], 0.4f);
  _gl_program->setUniformValue(_uniformLocations[Shininess], 1.f);

  glDrawArrays(GL_TRIANGLES, 0, _plane->ntris);
  _plane->vao->release();

  _gl_program->release();
} // RenderWidget::paintGL

void RenderWidget::enterEvent(QEvent*) {
  setFocus();
} // RenderWidget::enterEvent

void RenderWidget::leaveEvent(QEvent* event) {
  if (!_ctrlDown) {
    clearFocus();
  } else {
    event->ignore();
  }
} // RenderWidget::leaveEvent

void RenderWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Control) {
    _ctrlDown = true;
  } else {
    QOpenGLWidget::keyReleaseEvent(event);
  }
} // RenderWidget::keyPressEvent

void RenderWidget::keyReleaseEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Control) {
    _ctrlDown = false;
  } else {
    QOpenGLWidget::keyReleaseEvent(event);
  }
} // RenderWidget::keyReleaseEvent

void RenderWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton ||
      (event->button() == Qt::LeftButton && _ctrlDown)) {
    _mouseDrag = true;
    _prevMousePos = event->localPos();
  }
} // RenderWidget::mousePressEvent

void RenderWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton ||
      (event->button() == Qt::LeftButton && _ctrlDown)) {
    _mouseDrag = false;
    _prevMousePos = event->localPos();
  }
} // RenderWidget::mouseReleaseEvent

void RenderWidget::mouseMoveEvent(QMouseEvent* event) {
  if (!_mouseDrag) {
    event->ignore();
    return;
  }

  QPointF const delta = event->localPos() - _prevMousePos;

  _root.rotate(static_cast<float>(delta.x()) * .1f, QVector3D(0.f, 1.f, 0.f));
  _root.rotate(static_cast<float>(delta.y()) * .1f, QVector3D(1.f, 0.f, 0.f));

  _prevMousePos = event->localPos();
} // RenderWidget::mouseMoveEvent

void RenderWidget::wheelEvent(QWheelEvent* event) {
  _camera.translate(0.f, 0.f, -event->angleDelta().y() / 1000.f);
} // RenderWidget::wheelEvent

void RenderWidget::messageLogged(QOpenGLDebugMessage const& msg) {
  QString error;

  switch (msg.severity()) {
  case QOpenGLDebugMessage::NotificationSeverity: error += "--"; break;
  case QOpenGLDebugMessage::HighSeverity: error += "!!"; break;
  case QOpenGLDebugMessage::MediumSeverity: error += "!~"; break;
  case QOpenGLDebugMessage::LowSeverity: error += "~~"; break;
  case QOpenGLDebugMessage::InvalidSeverity: error += ".."; break;
  case QOpenGLDebugMessage::AnySeverity: error += "??"; break;
  }

  error += " [";
  switch (msg.source()) {
  case QOpenGLDebugMessage::APISource: error += "API"; break;
  case QOpenGLDebugMessage::WindowSystemSource: error += "WindowSystem"; break;
  case QOpenGLDebugMessage::ShaderCompilerSource:
    error += "ShaderCompiler";
    break;
  case QOpenGLDebugMessage::ThirdPartySource: error += "ThirdParty"; break;
  case QOpenGLDebugMessage::ApplicationSource: error += "Application"; break;
  case QOpenGLDebugMessage::OtherSource: error += "Other"; break;
  case QOpenGLDebugMessage::InvalidSource: error += "Invalid"; break;
  case QOpenGLDebugMessage::AnySource: error += "Any"; break;
  }

  error += ":";
  switch (msg.type()) {
  case QOpenGLDebugMessage::ErrorType: error += "Error"; break;
  case QOpenGLDebugMessage::DeprecatedBehaviorType:
    error += "DeprecatedBehavior";
    break;
  case QOpenGLDebugMessage::UndefinedBehaviorType:
    error += "UndefinedBehavior";
    break;
  case QOpenGLDebugMessage::PortabilityType: error += "Portability"; break;
  case QOpenGLDebugMessage::PerformanceType: error += "Performance"; break;
  case QOpenGLDebugMessage::OtherType: error += "Other"; break;
  case QOpenGLDebugMessage::MarkerType: error += "Marker"; break;
  case QOpenGLDebugMessage::GroupPushType: error += "GroupPush"; break;
  case QOpenGLDebugMessage::GroupPopType: error += "GroupPop"; break;
  case QOpenGLDebugMessage::InvalidType: error += "Invalid"; break;
  case QOpenGLDebugMessage::AnyType: error += "Any"; break;
  }

  error += "]:";
  qInfo() << qPrintable(error) << qPrintable(msg.message());
} // RenderWidget::messageLogged

void RenderWidget::update() {
  QOpenGLWidget::update();
} // RenderWidget::update

bool RenderWidget::genSurface(float* pVolume, int /*timestep*/, int /*column*/,
                              float isovalue, int xsize, int ysize, int zsize,
                              std::vector<QVector3D>& vbo) {

  cl_uint const gridSize[4] = {static_cast<cl_uint>(_model->dimX()),
                               static_cast<cl_uint>(_model->dimY()),
                               static_cast<cl_uint>(_model->dimZ()), 0};

  uint const numVoxels =
    (gridSize[0] - 1) * (gridSize[1] - 1) * (gridSize[2] - 1);
  uint const nSites = xsize * ysize * zsize;

  // maximum number of allowed vertices (also set is kernels/isosurface.cl)
  uint const maxVerts = (MAX_VERTS_PER_CELL * numVoxels);

  unsigned int const memSize = 4 * sizeof(float) * maxVerts;

  size_t const offset[3] = {0, 0, 0};
  size_t const region[3] = {gridSize[0], gridSize[1], gridSize[2]};

  size_t const localSize = NTHREADS;
  size_t globalSize;

  cl_image_format volumeFormat = {CL_R, CL_FLOAT};
  cl_image_format gradientFormat = {CL_RGBA, CL_FLOAT};

  cl_int ec;
  cl_event copy_event, run_event;
  std::vector<cl_event> finish_events(2);

  cl_mem d_volume = 0;
  cl_mem d_grad_array = 0;
  cl_mem d_gradient = 0;
  cl_mem d_vertices = 0;
  cl_mem d_normals = 0;

  std::vector<float> h_vertices(maxVerts * 4), h_normals(maxVerts * 4);
  size_t num_vertices = 0;
  bool ret = false;

  d_volume = clCreateImage3D(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                             &volumeFormat, gridSize[0], gridSize[1],
                             gridSize[2], 0, 0, pVolume, &ec);
  if (ec != CL_SUCCESS) {
    qCritical(QString("allocating volume buffer failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  d_grad_array = clCreateBuffer(_context, CL_MEM_READ_WRITE,
                                sizeof(cl_float4) * nSites, 0, &ec);
  if (ec != CL_SUCCESS) {
    qCritical(QString("allocating gradient buffer failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  // CREATE THE GRADIENT ARRAY
  // Create kernel arguments
  ec = clSetKernelArg(_grad_kernel, 0, sizeof(cl_mem), &d_volume);
  ec |= clSetKernelArg(_grad_kernel, 1, sizeof(cl_mem), &d_grad_array);
  ec |= clSetKernelArg(_grad_kernel, 2, 4 * sizeof(cl_uint), gridSize);
  if (ec != CL_SUCCESS) {
    qCritical(QString("setting gradient kernel arguments failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  // Enqueue the kernel
  globalSize =
    ((gridSize[0] * gridSize[1] * gridSize[2]) / localSize + 1) * localSize;

  ec = clEnqueueNDRangeKernel(_queue, _grad_kernel, 1, Q_NULLPTR, &globalSize,
                              &localSize, 0, Q_NULLPTR, &run_event);
  if (ec != CL_SUCCESS) {
    qCritical(QString("enqueuing gradient kernel failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  d_gradient =
    clCreateImage3D(_context, CL_MEM_READ_ONLY, &gradientFormat, gridSize[0],
                    gridSize[1], gridSize[2], 0, 0, Q_NULLPTR, &ec);
  if (ec) {
    qCritical(QString("allocating gradient texture failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  // Copy the gradient buffer into the gradient texture
  ec = clEnqueueCopyBufferToImage(_queue, d_grad_array, d_gradient, 0, offset,
                                  region, 1, &run_event, &copy_event);
  if (ec != CL_SUCCESS) {
    qCritical(QString("copying gradient buffer failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  clReleaseMemObject(d_grad_array);
  d_grad_array = 0;

  d_vertices = clCreateBuffer(_context, CL_MEM_READ_WRITE, memSize, 0, &ec);
  if (ec != CL_SUCCESS) {
    qCritical(QString("allocating vertex buffer failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  d_normals = clCreateBuffer(_context, CL_MEM_READ_WRITE, memSize, 0, &ec);
  if (ec != CL_SUCCESS) {
    qCritical(QString("allocating normal buffer failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  // CREATE THE MESH
  // Create the kernel arguments
  ec = clSetKernelArg(_iso_kernel, 0, sizeof(cl_mem), &d_vertices);
  ec |= clSetKernelArg(_iso_kernel, 1, sizeof(cl_mem), &d_normals);
  ec |= clSetKernelArg(_iso_kernel, 2, sizeof(cl_mem), &d_volume);
  ec |= clSetKernelArg(_iso_kernel, 3, sizeof(cl_mem), &d_gradient);
  ec |= clSetKernelArg(_iso_kernel, 4, sizeof(cl_uint) * 4, gridSize);
  ec |= clSetKernelArg(_iso_kernel, 5, sizeof(uint), &numVoxels);
  ec |= clSetKernelArg(_iso_kernel, 6, sizeof(float), &isovalue);
  ec |= clSetKernelArg(_iso_kernel, 7, sizeof(cl_mem), &_numVertsTable);
  ec |= clSetKernelArg(_iso_kernel, 8, sizeof(cl_mem), &_triTable);
  if (ec != CL_SUCCESS) {
    qCritical(QString("setting isosurface kernel arguments failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  // Enqueue kernel
  globalSize = (numVoxels / localSize + 1) * localSize;
  ec = clEnqueueNDRangeKernel(_queue, _iso_kernel, 1, Q_NULLPTR, &globalSize,
                              &localSize, 1, &copy_event, &run_event);
  if (ec != CL_SUCCESS) {
    qCritical(QString("enqueue isosurface kernel failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  // Read the vertices array
  ec = clEnqueueReadBuffer(_queue, d_vertices, CL_FALSE, 0, memSize,
                           h_vertices.data(), 1, &run_event, &finish_events[0]);
  if (ec != CL_SUCCESS) {
    qCritical(QString("reading vertex buffer failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  // Read the normals array
  ec = clEnqueueReadBuffer(_queue, d_normals, CL_FALSE, 0, memSize,
                           h_normals.data(), 1, &run_event, &finish_events[1]);
  if (ec != CL_SUCCESS) {
    qCritical(QString("reading normal buffer failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  ec = clWaitForEvents(static_cast<cl_uint>(finish_events.size()),
                       finish_events.data());
  if (ec != CL_SUCCESS) {
    qCritical(QString("reading buffers failed: %1")
                .arg(clstrerror(ec))
                .toLocal8Bit()
                .constData());
    goto err;
  }

  // compact the mesh vertices
  for (size_t i = 0; i < maxVerts * 4; i += 4) {
    if (h_vertices[i + 3] > 0.f) num_vertices += 1;
  }

  vbo.resize(num_vertices * 2);

  for (size_t i = 0, pos = 0; i < maxVerts * 4; i += 4) {
    if (h_vertices[i + 3] > 0.f) {
      vbo[pos] = QVector3D(h_vertices[i], h_vertices[i + 1], h_vertices[i + 2]);
      vbo[pos + 1] =
        QVector3D(-h_normals[i], -h_normals[i + 1], -h_normals[i + 2]);
      pos += 2;
    }
  }

  ret = true;
err:
  if (d_grad_array) clReleaseMemObject(d_grad_array);
  if (d_volume) clReleaseMemObject(d_volume);
  if (d_gradient) clReleaseMemObject(d_gradient);
  if (d_vertices) clReleaseMemObject(d_vertices);
  if (d_normals) clReleaseMemObject(d_normals);

  return ret;
} // RenderWidget::genSurface

RenderWidget::Drawable::Drawable()
  : vao(Q_NULLPTR)
  , vbo(Q_NULLPTR)
  , ntris(0)
  , color(Qt::white)
  , mode(GL_FILL)
  , visible(false) {
} // RenderWidget::Drawable::Drawable

RenderWidget::Drawable::Drawable(std::vector<QVector3D> const& data,
                                 GLsizei ntris_, QColor const& color_,
                                 float scale)
  : vao(new QOpenGLVertexArrayObject)
  , vbo(new QOpenGLBuffer)
  , ntris(ntris_)
  , color(color_)
  , mode(GL_FILL)
  , visible(true) {
  vbo->create();
  vbo->bind();
  vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
  vbo->allocate(data.data(), data.size() * sizeof(QVector3D));

  vao->create();
  vao->bind();

  worldFromModel.scale(scale);
  worldFromModel.translate(-.5f, -.5f, -.5f);
} // RenderWidget::Drawable::Drawable

RenderWidget::Drawable::~Drawable() {
  if (vbo) {
    vbo->destroy();
    delete vbo;
  }
  if (vao) {
    vao->destroy();
    delete vao;
  }
} // RenderWidget::Drawable::~Drawable

