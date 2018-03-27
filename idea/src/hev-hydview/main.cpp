#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("HPCVG");
  QCoreApplication::setApplicationName("hydview");
  QCoreApplication::setApplicationVersion(QT_VERSION_STR);

  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::applicationName());
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("dir", "The directory to open.");
  parser.process(app);

  MainWindow mainWindow;
  if (!parser.positionalArguments().isEmpty()) {
  }
  mainWindow.show();
  return app.exec();
}

