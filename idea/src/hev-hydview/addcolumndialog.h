#ifndef HEV_HYDVIEW_ADDCOLUMNDIALOG_H
#define HEV_HYDVIEW_ADDCOLUMNDIALOG_H

#include <QDialog>
#include "datamodel.h"

namespace Ui {
  class AddColumnDialog;
}; // namespace Ui

class QStringListModel;

class AddColumnDialog : public QDialog {
  Q_OBJECT

public:
  AddColumnDialog(QWidget* parent = Q_NULLPTR);
  ~AddColumnDialog();

  void setColumn1Model(QStringListModel*);
  void setColumn2Model(QStringListModel*);

  int column1Index();
  int column2Index();

  DataModel::Operation operation();

private:
  Ui::AddColumnDialog* _ui;
}; // class AddColumnDialog

#endif // HEV_HYDVIEW_ADDCOLUMNDIALOG_H

