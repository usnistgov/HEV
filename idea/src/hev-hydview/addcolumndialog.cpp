#include "addcolumndialog.h"
#ifdef NO_HEV
#include "ui_addcolumndialog.h"
#else
#include "addcolumndialog.ui.h"
#endif
#include "datamodel.h"
#include <QStringListModel>

AddColumnDialog::AddColumnDialog(QWidget* parent)
  : QDialog(parent), _ui(new Ui::AddColumnDialog) {
  // set up UI
  _ui->setupUi(this);

  QStringList operations;
  operations << "Add (+)" << "Multiply (*)";
  QStringListModel *model = new QStringListModel(operations);
  _ui->comboBoxOperation->setModel(model);
} // AddColumnDialog::AddColumnDialog

AddColumnDialog::~AddColumnDialog() {
} // AddColumnDialog::~AddColumnDialog

void AddColumnDialog::setColumn1Model(QStringListModel* model) {
  _ui->comboBoxColumn1->setModel(model);
} // AddColumnDialog::setColumn1Model

void AddColumnDialog::setColumn2Model(QStringListModel* model) {
  _ui->comboBoxColumn2->setModel(model);
} // AddColumnDialog::setColumn2Model

int AddColumnDialog::column1Index() {
  return _ui->comboBoxColumn1->currentIndex();
} // AddColumnDialog::column1

int AddColumnDialog::column2Index() {
  return _ui->comboBoxColumn2->currentIndex();
} // AddColumnDialog::column2

DataModel::Operation AddColumnDialog::operation() {
  // eww
  QString operation = _ui->comboBoxOperation->currentText();
  if (operation == "Add (+)") {
    return DataModel::Plus;
  } else if (operation == "Multiply (*)") {
    return DataModel::Multiply;
  }
#if _WIN32
  __assume(0);
#else
  __builtin_unreachable();
#endif
} // AddColumnDialog::operation

