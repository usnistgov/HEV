#include "mainwindow.h"
#ifdef NO_HEV
#include "ui_mainwindow.h"
#else
#include "mainwindow.ui.h"
#endif
#include "addcolumndialog.h"
#include "clipcontroldialog.h"
#ifndef NO_HEV
#include "irisprocess.h"
#else
#include "renderwidget.h"
#endif
#include <qwt_plot.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>
#include <qwt_curve_fitter.h>
#include <QStandardItemModel>
#include <QtWidgets>
#include <QtDebug>

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , _ui(new Ui::MainWindow)
  , _isosurfacesModel(new QStandardItemModel(0, 4, this))
  , _isosurfacesMapper(new QSignalMapper(this))
  , _addColumnDialog(new AddColumnDialog(this))
  , _clipControlDialog(Q_NULLPTR)
#ifdef NO_HEV
  , _renderWidget(new RenderWidget(this))
#endif
  , _currColor(Qt::blue) {
  // set up UI
  _ui->setupUi(this);
  QPalette palette = _ui->pushButtonColor->palette();
  palette.setColor(_ui->pushButtonColor->backgroundRole(), _currColor);
  _ui->pushButtonColor->setPalette(palette);

  QStringList headerLabels;
  headerLabels << "V" << "C" << "Iso" << "Data";
  _isosurfacesModel->setHorizontalHeaderLabels(headerLabels);
  _ui->tableViewIsosurfaces->setModel(_isosurfacesModel);
  _ui->tableViewIsosurfaces->resizeColumnsToContents();
  connect(_ui->tableViewIsosurfaces->selectionModel(),
          &QItemSelectionModel::selectionChanged, this,
          &MainWindow::isosurfacesSelectionChanged);
  connect(_isosurfacesMapper, SIGNAL(mapped(int)), this, SLOT(clicked(int)));

#ifndef NO_HEV
  _irisThread.setObjectName("hydviewIrisComm");
  _iris = new IrisProcess;
  _iris->moveToThread(&_irisThread);
  connect(this, &MainWindow::startIris, _iris, &IrisProcess::startIris);
  connect(_iris, &IrisProcess::started, this, &MainWindow::irisStarted);
  connect(this, &MainWindow::init, _iris, &IrisProcess::init);
  connect(_iris, &IrisProcess::initDone, this, &MainWindow::initDone);
  connect(this, &MainWindow::generate, _iris, &IrisProcess::generate);
  connect(_iris, &IrisProcess::generateDone, this, &MainWindow::generateDone);
  connect(this, &MainWindow::stopIris, _iris, &IrisProcess::stopIris);
  _irisThread.start();
#else
  _renderWidget->setMinimumWidth(700);
  reinterpret_cast<QGridLayout*>(centralWidget()->layout())
    ->addWidget(_renderWidget, 0, 6, 4, 1);
  connect(_renderWidget, &RenderWidget::generateDone, this,
          &MainWindow::generateDone);
  connect(this, &MainWindow::generate, _renderWidget, &RenderWidget::generate);
#endif

  _modelThread.setObjectName("hydviewDataLoader");
  _model = new DataModel;
  _model->moveToThread(&_modelThread);
  connect(this, &MainWindow::load, _model, &DataModel::load);
  connect(_model, &DataModel::loaded, this, &MainWindow::loaded);
  connect(this, &MainWindow::insert, _model, &DataModel::insert);
  connect(_model, &DataModel::inserted, this, &MainWindow::inserted);
  _modelThread.start();

  // read settings
  QSettings settings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());

  QByteArray const geometry =
    settings.value("geometry", QByteArray()).toByteArray();
  if (!geometry.isEmpty()) restoreGeometry(geometry);

  _currDirName = settings.value("lastDir", QString()).toString();

  int const customCount = settings.beginReadArray("custom-colors");
  for (int i = 0; i < customCount; ++i) {
    settings.setArrayIndex(i);
    QColorDialog::setCustomColor(i, settings.value("color").value<QColor>());
  }
  settings.endArray();

  setUnifiedTitleAndToolBarOnMac(true);

#ifndef NO_HEV
  emit startIris();
#else
  _ui->actionOpen->setEnabled(true);
#endif
} // MainWindow::MainWindow

MainWindow::~MainWindow() {
#ifndef NO_HEV
  emit stopIris();
  _irisThread.quit();
  _irisThread.wait();
#endif
  _modelThread.quit();
  _modelThread.wait();
} // MainWindow::~MainWindow

void MainWindow::closeEvent(QCloseEvent* event) {
  // write settings
  QSettings settings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());
  settings.setValue("geometry", saveGeometry());
  settings.setValue("lastDir", _currDirName);

  settings.beginWriteArray("custom-colors");
  for (int i = 0; i < QColorDialog::customCount(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("color", QColorDialog::customColor(i));
  }
  settings.endArray();

  event->accept();
} // MainWindow::closeEvent

void MainWindow::on_actionOpen_triggered() {
  QString const dirName =
    QFileDialog::getExistingDirectory(this, tr("Open Directory"), _currDirName);
  if (dirName.isEmpty()) return;

  QDir dir(dirName);
  dir.makeAbsolute();
  if (!dir.exists()) return;

  statusBar()->showMessage(tr("Opening %1").arg(dir.path()));
  QApplication::setOverrideCursor(Qt::WaitCursor);
  _ui->actionOpen->setEnabled(false);
  emit load(dir.path());
} // MainWindow::on_actionOpen_triggered

void MainWindow::on_actionAdd_triggered() {
  if (!_model) return;
  int code = _addColumnDialog->exec();
  if (code != QDialog::Accepted) return;

  QStringListModel* columns = _model->items();

  int column1Index = _addColumnDialog->column1Index();
  QString column1Name =
    columns->data(columns->index(column1Index), Qt::DisplayRole).toString();
  int column2Index = _addColumnDialog->column2Index();
  QString column2Name =
    columns->data(columns->index(column2Index), Qt::DisplayRole).toString();

  DataModel::Operation operation = _addColumnDialog->operation();
  QString operationName;

  switch (operation) {
  case DataModel::Plus: operationName = "+"; break;
  case DataModel::Subtract: operationName = "-"; break;
  case DataModel::Multiply: operationName = "*"; break;
  case DataModel::Divide: operationName = "/"; break;
  }

  columns->insertRows(columns->rowCount(), 1);
  columns->setData(
    columns->index(columns->rowCount() - 1),
    QString("%1 %2 %3").arg(column1Name).arg(operationName).arg(column2Name));

  QApplication::setOverrideCursor(Qt::WaitCursor);
  emit insert(column1Index, column2Index, operation);
} // MainWindow::on_actionAdd_triggered

void MainWindow::on_actionClip_triggered() {
  if (!_clipControlDialog) {
    _clipControlDialog = new ClipControlDialog(this);
#ifdef NO_HEV
    connect(_clipControlDialog, &ClipControlDialog::valueChanged,
            _renderWidget, &RenderWidget::clipPlaneChanged);
#endif
  }
  _clipControlDialog->show();
  _clipControlDialog->raise();
  _clipControlDialog->activateWindow();
} // MainWindow::on_actionClip_triggered

#ifndef NO_HEV
void MainWindow::irisStarted() {
  _ui->actionOpen->setEnabled(true);
  _iris->send("LOAD clip.mcp");
  _iris->send("LOAD backgroundBlack.mcp");
  _iris->send("ECHO ON");
} // MainWindow::irisStarted
#endif

void MainWindow::loaded(QString const& dirName, QString const& err) {
  if (!err.isEmpty()) {
    QApplication::restoreOverrideCursor();
    _ui->actionOpen->setEnabled(true);
    QMessageBox::critical(this, tr("Invalid directory"), err);
    return;
  }

  _currDirName = dirName;
#ifndef NO_HEV
  emit init(_model->shm_name());
#else
  initDone("");
#endif
}

void MainWindow::initDone(QString const& err) {
  if (!err.isEmpty()) {
    QApplication::restoreOverrideCursor();
    _ui->actionOpen->setEnabled(true);
    QMessageBox::critical(this, tr("Invalid directory"), err);
    return;
  }

  setActionsEnabled(true);
  _ui->spinBoxTimestep->setMaximum(static_cast<int>(_model->nTimesteps()) - 1);

  _plotHist = new QwtPlot(this);
  _plotHist->canvas()->setPalette(QPalette(QColor::fromRgb(255, 255, 255)));
  _plotHist->setAxisTitle(QwtPlot::xBottom, tr("Bins"));
  _plotHist->setAxisTitle(QwtPlot::yLeft, tr("Count"));

  _hist = new QwtPlotHistogram("Histogram");
  _hist->setBrush(QBrush(QColor::fromRgb(32, 64, 200)));
  _hist->attach(_plotHist);

  _plotCurv = new QwtPlot(this);
  _plotCurv->canvas()->setPalette(QPalette(QColor::fromRgb(255, 255, 255)));
  _plotCurv->setAxisTitle(QwtPlot::xBottom, tr("Timestep"));
  _plotCurv->setAxisTitle(QwtPlot::yLeft, tr("Average"));

  _curv = new QwtPlotCurve("Average");
  _curv->attach(_plotCurv);

  QGridLayout* grid = reinterpret_cast<QGridLayout*>(centralWidget()->layout());
  grid->addWidget(_plotHist, 2, 0, 1, 3);
  grid->addWidget(_plotCurv, 3, 0, 1, 3);

  // This has to happen after creating _hist and _curv
  _ui->comboBoxData->setModel(_model->items());
  _addColumnDialog->setColumn1Model(_model->items());
  _addColumnDialog->setColumn2Model(_model->items());
  _ui->comboBoxData->setCurrentIndex(0);

#ifndef NO_HEV
  _iris->send(QString("NAV POSITION %1 %2 %3")
                .arg(_model->dimX() * -0.5)
                .arg(_model->dimY() * 1.25)
                .arg(_model->dimZ() * -0.5)
                .toLocal8Bit()
                .constData());
  _iris->send(QString("NAV PIVOTPOINT %1 %2 %3")
                .arg(_model->dimX() / 2)
                .arg(_model->dimY() / 2)
                .arg(_model->dimZ() / 2)
                .toLocal8Bit()
                .constData());
#else
  _renderWidget->setModel(_model);
#endif

  QApplication::restoreOverrideCursor();
  _ui->actionOpen->setEnabled(true);
  statusBar()->showMessage(tr("Opened %1").arg(_currDirName));
} // MainWindow::initDone

void MainWindow::generateDone(QString const& err) {
  QApplication::restoreOverrideCursor();
  setActionsEnabled(true);

  if (!err.isEmpty()) {
    QMessageBox::critical(this, tr("Unable to generate isosurface"), err);
    return;
  }

  QStringListModel* columns = _model->items();
  int rowIdx = _isosurfacesModel->rowCount();

  QPixmap pixmap(10, 10);
  pixmap.fill(_currColor);

  QList<QStandardItem*> row;
  row.append(new QStandardItem(""));
  row.append(new QStandardItem(QIcon(pixmap), ""));
  row.append(new QStandardItem(
    QString("%1").arg(_ui->horizontalSliderIsovalue->value() / 1000.f)));
  row.append(new QStandardItem(
    columns
      ->data(columns->index(_ui->comboBoxData->currentIndex()), Qt::DisplayRole)
      .toString()));

  _isosurfacesModel->insertRow(rowIdx, row);

  QCheckBox* checkBoxVisible = new QCheckBox("", _ui->tableViewIsosurfaces);
  checkBoxVisible->setChecked(true);
  _ui->tableViewIsosurfaces->setIndexWidget(_isosurfacesModel->index(rowIdx, 0),
                                            checkBoxVisible);
  connect(checkBoxVisible, SIGNAL(clicked()), _isosurfacesMapper, SLOT(map()));
  _isosurfacesMapper->setMapping(checkBoxVisible, rowIdx);

  _ui->tableViewIsosurfaces->resizeColumnsToContents();

  statusBar()->showMessage(tr("Generated isosurface"));
} // MainWindow::generateDone

void MainWindow::inserted(QString const& err) {
  QApplication::restoreOverrideCursor();

  if (!err.isEmpty()) {
    QMessageBox::critical(this, tr("Unable to insert new column"), err);
    return;
  }

#ifndef NO_HEV
  _iris->send("DSO hydview INSERTED");
#endif

  _ui->comboBoxData->setCurrentIndex(_model->items()->rowCount() - 1);
  statusBar()->showMessage(tr("Inserted new column"));
}// MainWindow::inserted

void MainWindow::on_comboBoxData_currentIndexChanged(int index) {
  _hist->setSamples(_model->histogram(static_cast<size_t>(index),
                                      static_cast<size_t>(_ui->spinBoxTimestep->value())));
  _curv->setSamples(_model->curve(static_cast<size_t>(index)));

  _plotHist->replot();
  _plotCurv->replot();
} // MainWindow::on_comboBoxData_currentIndexChanged

void MainWindow::on_spinBoxTimestep_valueChanged(int timestep) {
  _hist->setSamples(
    _model->histogram(static_cast<size_t>(_ui->comboBoxData->currentIndex()),
                      static_cast<size_t>(timestep)));
  _plotHist->replot();

#ifndef NO_HEV
  _iris->send(
    QString("DSO hydview TIMESTEP %1").arg(timestep).toLocal8Bit().constData());
#else
  _renderWidget->timestepChanged(timestep);
#endif
} // MainWindow::on_spinBoxTimestep_valueChanged

void MainWindow::on_horizontalSliderIsovalue_valueChanged(int value) {
  float isoValue = static_cast<float>(value) / 1000.f;
  _ui->lineEditIsovalueValue->setText(
    tr("%1").arg(static_cast<double>(isoValue)));
} // MainWindow::on_horizontalSliderIsovalue_valueChanged

void MainWindow::on_lineEditIsovalueValue_editingFinished() {
  float isoValue = _ui->lineEditIsovalueValue->text().toFloat() * 1000.f;
  _ui->horizontalSliderIsovalue->setValue(isoValue);
} // MainWindow::on_lineEditIsovalueValue_editingFinished

void MainWindow::isosurfacesSelectionChanged(QItemSelection const& selected,
                                             QItemSelection const&) {
  _ui->pushButtonDelete->setEnabled(!selected.empty());
} // MainWindow::isosurfacesSelectionChanged

void MainWindow::on_pushButtonColor_clicked(bool) {
  _currColor = QColorDialog::getColor(_currColor, this, "Isosurface Color");

  QPalette palette = _ui->pushButtonColor->palette();
  palette.setColor(_ui->pushButtonColor->backgroundRole(), _currColor);
  _ui->pushButtonColor->setPalette(palette);
} // MainWindow::on_pushButtonColor_clicked

void MainWindow::on_pushButtonGenerate_clicked(bool) {
  QApplication::setOverrideCursor(Qt::WaitCursor);
  setActionsEnabled(false);

  emit generate(_ui->spinBoxTimestep->value(),
                _ui->comboBoxData->currentIndex(),
                _ui->horizontalSliderIsovalue->value() / 1000.f, _currColor);
} // MainWindow::on_pushButtonGenerate_clicked

void MainWindow::on_pushButtonDelete_clicked(bool) {
  QModelIndexList selected =
    _ui->tableViewIsosurfaces->selectionModel()->selectedRows();
  int const rowIdx = selected[0].row();
#ifndef NO_HEV
  int const colRowIdx = _model->items()->stringList().indexOf(
    _isosurfacesModel->item(rowIdx, 2)->data(Qt::DisplayRole).toString());
  _iris->send(QString("DSO hydview REMOVE %1").arg(colRowIdx));
#else
  _renderWidget->remove(rowIdx);
#endif
  _isosurfacesModel->removeRows(rowIdx, 1);
} // MainWindow::on_pushButtonDelete_clicked

void MainWindow::clicked(int rowIdx) {
  QCheckBox* checkBox =
    dynamic_cast<QCheckBox*>(_ui->tableViewIsosurfaces->indexWidget(
      _isosurfacesModel->index(rowIdx, 0)));
#ifndef NO_HEV
  int const colRowIdx = _model->items()->stringList().indexOf(
    _isosurfacesModel->item(rowIdx, 3)->data(Qt::DisplayRole).toString());
  _iris->send(QString("DSO hydview NODEMASK %1 %2 %3")
                .arg(_ui->spinBoxTimestep->value())
                .arg(colRowIdx)
                .arg(checkBox->isChecked() ? "ON" : "OFF"));
#else
  _renderWidget->setVisible(rowIdx, checkBox->isChecked());
#endif
} // MainWindow::clicked

void MainWindow::setActionsEnabled(bool enabled) {
  _ui->actionAdd->setEnabled(enabled);
  _ui->actionClip->setEnabled(enabled);
  _ui->comboBoxData->setEnabled(enabled);
  _ui->spinBoxTimestep->setEnabled(enabled);
  _ui->horizontalSliderIsovalue->setEnabled(enabled);
  _ui->lineEditIsovalueValue->setEnabled(enabled);
  _ui->pushButtonColor->setEnabled(enabled);
  _ui->pushButtonGenerate->setEnabled(enabled);
} // MainWindow::setActionsEnabled

