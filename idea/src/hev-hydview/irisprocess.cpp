#include "irisprocess.h"
#include <QColor>
#include <QProcess>
#include <QThread>

IrisProcess::IrisProcess(QObject* parent)
  : QObject(parent)
  , _process(new QProcess(this))
  , _send(getenv("IRIS_CONTROL_FIFO"))
  , _reply(QString("/tmp/hydviewFifo-%1").arg(getenv("USER")).toStdString()) {

  _process->setProcessChannelMode(QProcess::MergedChannels);
  getcwd(_buffer, PIPE_BUF);
  _process->setWorkingDirectory(_buffer);
  _process->setProgram("irisfly");
  _process->setArguments(QStringList("hydview-desktop.iris"));

  connect(_process, &QProcess::started, this, &IrisProcess::processStarted);
  connect(_process, &QProcess::readyRead, this, &IrisProcess::processOutput);
} // IrisProcess::IrisProcess

IrisProcess::~IrisProcess() {
  delete _process;
} // IrisProcess::~IrisProcess

bool IrisProcess::readReply(std::string& rep) {
  return _reply.readLine(&rep);
} // IrisProcess::readReply

void IrisProcess::startIris() {
  _process->start();
  _reply.unlinkOnExit();
  _reply.open();
} // IrisProcess::startIris

void IrisProcess::stopIris() {
  if (_process->state() == QProcess::NotRunning) return;
  send("TERMINATE");
  _process->waitForFinished();
} // IrisProcess::stop

void IrisProcess::init(QString const& shm_name) {
  send(QString("DSO hydview INIT %1").arg(shm_name));
  _reply.readLine(&_line);
  emit initDone(_line.c_str());
} // IrisProcess::init

void IrisProcess::generate(int timestep, int column, float isoValue,
                           QColor const& color) {
  QString cmd = QString("DSO hydview GENERATE %1 %2 %3 %4 %5 %6")
                  .arg(timestep)
                  .arg(column)
                  .arg(isoValue)
                  .arg(color.redF())
                  .arg(color.greenF())
                  .arg(color.blueF());

  send(cmd);
  _reply.readLine(&_line);
  emit generateDone(_line.c_str());
} // IrisProcess::generate

void IrisProcess::send(QString const& cmd) {
  _send.write(cmd.toLocal8Bit().constData());
  _send.write("\n");
} // IrisProcess::send

void IrisProcess::processStarted() {
  _send.open(O_WRONLY);
  emit started();
} // IrisProcess::processStarted

void IrisProcess::processOutput() {
  qint64 nRead;
  do {
    nRead = _process->read(_buffer, PIPE_BUF);
    write(STDOUT_FILENO, _buffer, nRead);
  } while (nRead > 0);
} // IrisProcess::processOutput

