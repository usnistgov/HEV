#include "clipcontroldialog.h"
#include "joystickwidget.h"
#include <QCloseEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#ifndef NO_HEV
#include <dtk.h>
#include <osg/Matrix>
#include <iris/Utils.h>
#endif

ClipControlDialog::ClipControlDialog(QWidget* parent)
  : QDialog(parent)
  , _joystickPosition(new JoystickWidget(this))
  , _joystickRotation(new JoystickWidget(this)) {
  connect(_joystickPosition, &JoystickWidget::positionChanged, this,
          &ClipControlDialog::positionChanged);
  connect(_joystickRotation, &JoystickWidget::positionChanged, this,
          &ClipControlDialog::rotationChanged);

  QVBoxLayout* box = new QVBoxLayout(this);
  box->addWidget(new QLabel("Position"));
  box->addWidget(_joystickPosition);
  box->addWidget(new QLabel("Rotation"));
  box->addWidget(_joystickRotation);
  box->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(box);

  memset(_sceneArray, 0, 6 * sizeof(float));

#ifndef NO_HEV
  _buttons = new dtkSharedMem("buttons", 0);
  if (_buttons->isInvalid()) {
    qFatal(tr("Unable to open buttons shm").toLocal8Bit().constData());
  }

  _wand= new dtkSharedMem("wand", 0);
  if (_wand->isInvalid()) {
    qFatal(tr("Unable to open wand shm").toLocal8Bit().constData());
  }

  int const sz = _wand->getSize();
  if (sz != 6 * sizeof(float) && sz != 16 * sizeof(double)) {
    qFatal(tr("Invalid size for wand shm").toLocal8Bit().constData());
  }

  _wandIsCoord = (sz == 6 * sizeof(float));
#endif
} // ClipControlDialog::ClipControlDialog

ClipControlDialog::~ClipControlDialog() {
#ifndef NO_HEV
  delete _buttons;
  delete _wand;
#endif
} // ClipControlDialog::~ClipControlDialog

void ClipControlDialog::showEvent(QShowEvent* event) {
#ifndef NO_HEV
  unsigned char o;
  _buttons->read(&o);
  o = o & 0xfe;
  o = o | 1;
  _buttons->write(&o);
#endif
} // ClipControlDialog::showEvent

void ClipControlDialog::closeEvent(QCloseEvent* event) {
#ifndef NO_HEV
  unsigned char old;
  _buttons->read(&old);
  old = old & 0xfe;
  old = old | 0;
  _buttons->write(&old);
  event->accept();
#endif
} // ClipControlDialog::closeEvent

void ClipControlDialog::positionChanged(QPointF const& delta) {
  _sceneArray[0] += delta.x();
  _sceneArray[1] += -delta.y();
#ifndef NO_HEV
  writeWand();
#else
  emit valueChanged(_sceneArray[0], _sceneArray[1], _sceneArray[2],
                    _sceneArray[3], _sceneArray[4], _sceneArray[5]);
#endif
} // ClipControlDialog::positionChanged

void ClipControlDialog::rotationChanged(QPointF const& delta) {
  _sceneArray[3] += -delta.x();
  _sceneArray[4] += -delta.y();
#ifndef NO_HEV
  writeWand();
#else
  emit valueChanged(_sceneArray[0], _sceneArray[1], _sceneArray[2],
                    _sceneArray[3], _sceneArray[4], _sceneArray[5]);
#endif
} // ClipControlDialog::rotationChanged

#ifndef NO_HEV
void ClipControlDialog::writeWand() {
  if (_wandIsCoord) {
    _wand->write(_sceneArray);
  } else {
    osg::Matrix mat;
    mat.makeRotate(
      iris::EulerToQuat(_sceneArray[3], _sceneArray[4], _sceneArray[5]));
    mat.postMultTranslate(
      osg::Vec3(_sceneArray[0], _sceneArray[1], _sceneArray[2]));
    _wand->write(mat.ptr());
  }
} // ClipControlDialog::writeWand
#endif

