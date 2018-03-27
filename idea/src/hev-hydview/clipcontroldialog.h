#ifndef HEV_HYDVIEW_CLIPCONTROLDIALOG_H
#define HEV_HYDVIEW_CLIPCONTROLDIALOG_H

#include <QDialog>

class JoystickWidget;
#ifndef NO_HEV
class dtkSharedMem;
#endif
class QCloseEvent;
class QShowEvent;

class ClipControlDialog : public QDialog {
  Q_OBJECT

public:
  ClipControlDialog(QWidget* parent = Q_NULLPTR);
  ~ClipControlDialog();

signals:
  void valueChanged(float x, float y, float z, float h, float p, float r);

protected:
  virtual void showEvent(QShowEvent*);
  virtual void closeEvent(QCloseEvent*);

private slots:
  void positionChanged(QPointF const&);
  void rotationChanged(QPointF const&);

private:
#ifndef NO_HEV
  void writeWand();
#endif

  JoystickWidget* _joystickPosition;
  JoystickWidget* _joystickRotation;

  float _sceneArray[6];

#ifndef NO_HEV
  dtkSharedMem* _buttons;
  dtkSharedMem* _wand;
  bool _wandIsCoord;
#endif
}; // class ClipControlDialog

#endif // HEV_HYDVIEW_CLIPCONTROLDIALOG_H

