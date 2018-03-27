#include "joystickwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtDebug>

JoystickWidget::JoystickWidget(QWidget* parent)
  : QWidget(parent)
  , _dragging(false)
  , _last(100, 100)
  , _curr(100, 100) {
} // JoystickWidget::JoystickWidget

JoystickWidget::~JoystickWidget() {
} // JoystickWidget::~JoystickWidget

void JoystickWidget::paintEvent(QPaintEvent* event) {
  QPainter painter(this);

  painter.setPen(Qt::gray);
  painter.setBrush(Qt::white);
  painter.drawRect(QRectF(0.f, 0.f, 199.f, 199.f));

  int x = std::min(std::max(_curr.x(), 1), 198);
  int y = std::min(std::max(_curr.y(), 1), 198);

  painter.setPen(Qt::red);
  painter.drawLine(QLineF(x, 0.f, x, 199.f));
  painter.drawLine(QLine(0.f, y, 199.f, y));
} // JoystickWidget::paintEvent

void JoystickWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    _dragging = true;
    _last = _curr = event->pos();
    repaint();
    event->accept();
  }
} // JoystickWidget::mousePressEvent

void JoystickWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    _dragging = false;
    _curr = QPoint(100, 100);
    repaint();
    event->accept();
  }
} // JoystickWidget::mouseReleaseEvent

void JoystickWidget::mouseMoveEvent(QMouseEvent* event) {
  if (_dragging) {
    _curr = event->pos();
    repaint();

    QPointF delta((_curr.x() - _last.x()), (_curr.y() - _last.y()));
    _last = _curr;

    emit positionChanged(delta);
  }
} // JoystickWidget::mouseMoveEvent

