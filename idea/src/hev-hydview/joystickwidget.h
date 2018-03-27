#ifndef HEV_HYDVIEW_JOYSTICKWIDGET_H
#define HEV_HYDVIEW_JOYSTICKWIDGET_H

#include <QWidget>

class JoystickWidget : public QWidget {
  Q_OBJECT

public:
  JoystickWidget(QWidget* parent = Q_NULLPTR);
  virtual ~JoystickWidget();

  virtual QSize sizeHint() const { return QSize(200, 200); }

signals:
  void positionChanged(QPointF const& delta);

protected:
  virtual void paintEvent(QPaintEvent*);
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);

private:
  bool _dragging;
  QPoint _last, _curr;
}; // class JoystickWidget

#endif // HEV_HYDVIEW_JOYSTICKWIDGET_H

