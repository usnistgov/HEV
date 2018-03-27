#ifndef HEV_HYDVIEW_MAINWINDOW_H
#define HEV_HYDVIEW_MAINWINDOW_H

#include "datamodel.h"
#include <QMainWindow>
#include <QThread>
#include <QItemSelection>

namespace Ui {
  class MainWindow;
}; // namespace Ui

class AddColumnDialog;
class ClipControlDialog;
#ifndef NO_HEV
class IrisProcess;
#else
class RenderWidget;
#endif
class QListView;
class QStandardItemModel;
class QSignalMapper;
class QwtPlot;
class QwtPlotHistogram;
class QwtPlotCurve;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget* parent = Q_NULLPTR);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

signals:
#ifndef NO_HEV
  void startIris();
  void init(QString const& shm_name);
  void stopIris();
#endif
  void load(QString const&);
  void generate(int timestep, int column, float isoValue, QColor const& color);
  void insert(int, int, DataModel::Operation);

private slots:
  void on_actionOpen_triggered();
  void on_actionAdd_triggered();
  void on_actionClip_triggered();
  void on_comboBoxData_currentIndexChanged(int);
  void on_spinBoxTimestep_valueChanged(int);
  void on_horizontalSliderIsovalue_valueChanged(int);
  void on_lineEditIsovalueValue_editingFinished();
  void on_pushButtonColor_clicked(bool);
  void on_pushButtonGenerate_clicked(bool);
  void on_pushButtonDelete_clicked(bool);

  void isosurfacesSelectionChanged(QItemSelection const&,
                                   QItemSelection const&);
  void clicked(int);

#ifndef NO_HEV
  void irisStarted();
#endif
  void initDone(QString const&);
  void loaded(QString const&, QString const&);
  void generateDone(QString const&);
  void inserted(QString const&);

private:
  void setActionsEnabled(bool enabled);

  Ui::MainWindow* _ui;
#ifndef NO_HEV
  IrisProcess* _iris;
  QThread _irisThread;
#endif
  QThread _modelThread;
  DataModel* _model;
  QStandardItemModel* _isosurfacesModel;
  QSignalMapper* _isosurfacesMapper;
  AddColumnDialog* _addColumnDialog;
  ClipControlDialog* _clipControlDialog;
#ifdef NO_HEV
  RenderWidget* _renderWidget;
#endif
  QString _currDirName;
  QColor _currColor;
  QwtPlot* _plotHist;
  QwtPlotHistogram* _hist;
  QwtPlot* _plotCurv;
  QwtPlotCurve* _curv;
}; // class MainWindow

#endif // HEV_HYDVIEW_MAINWINDOW_H

