#ifndef HYDVIEW_RENDERWIDGET_H
#define HYDVIEW_RENDERWIDGET_H

#include "CL/opencl.h"
#include "transform3d.h"
#include "camera3d.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class DataModel;
class QOpenGLDebugMessage;
class QOpenGLDebugLogger;
class QOpenGLShaderProgram;
class QOpenGLShader;
class QOpenGLVertexArrayObject;
class QEvent;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT

public:
  RenderWidget(QWidget* parent = Q_NULLPTR);
  virtual ~RenderWidget();

public slots:
  void setModel(DataModel* model);
  void timestepChanged(int);

  void generate(int timestep, int column, float isoValue, QColor const& color);
  void remove(int row);
  void setVisible(int row, bool value);

  void clipPlaneChanged(float x, float y, float z, float h, float p, float r);

signals:
  void generateDone(QString err);

protected:
  void initializeGL();
  void initializeCL();
  void resizeGL(int, int);
  void paintGL();

  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void keyPressEvent(QKeyEvent*);
  void keyReleaseEvent(QKeyEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void wheelEvent(QWheelEvent*);

protected slots:
  void messageLogged(QOpenGLDebugMessage const&);
  void update();

private:
  bool genSurface(float* pVolume, int timestep, int column, float isovalue,
                  int xsize, int ysize, int zsize, std::vector<QVector3D>& vbo);

  DataModel* _model;
  int _timestep;

  QMatrix4x4 _projection;
  Transform3D _root;
  Camera3D _camera;
  bool _ctrlDown;
  bool _mouseDrag;
  QPointF _prevMousePos;

  cl_platform_id _platform;
  cl_device_id _device;
  cl_context _context;
  cl_command_queue _queue;
  cl_program _cl_program;
  cl_kernel _iso_kernel, _grad_kernel;

  cl_mem _triTable;
  cl_mem _numVertsTable;

  QOpenGLDebugLogger* _logger;
  QOpenGLShaderProgram* _gl_program;

  class Drawable {
  public:
    QOpenGLVertexArrayObject* vao;
    QOpenGLBuffer* vbo;
    GLsizei ntris;
    QColor color;
    Transform3D worldFromModel;
    QMatrix4x4 testMat;
    GLenum mode;
    bool visible;

    Drawable();
    Drawable(std::vector<QVector3D> const& data, GLsizei ntris_,
             QColor const& color_, float scale);
    ~Drawable();
  };

  Drawable* _plane;
  std::vector<std::vector<Drawable*>> _surfaces;

  enum {
    ProjectionFromCamera,
    CameraFromWorld,
    WorldFromModel,
    NormalMatrix,
    ClipPlane,
    CameraPosition,
    MColor,
    Refindex,
    Shininess,
    NumUniforms
  };

  std::vector<int> _uniformLocations;
}; // class RenderWidget

#endif // HYDVIEW_RENDERWIDGET_H

