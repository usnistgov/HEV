#ifndef HEV_HYDVIEW_IRISPROCESS_H
#define HEV_HYDVIEW_IRISPROCESS_H

#include <QObject>
#include <iris/FifoReader.h>
#include <limits.h>

class QProcess;

class IrisProcess : public QObject {
  Q_OBJECT

public:
  IrisProcess(QObject* parent = Q_NULLPTR);
  virtual ~IrisProcess();

  bool readReply(std::string& rep);

public slots:
  void startIris();
  void stopIris();

  void init(QString const& shm_name);
  void generate(int timestep, int column, float isoValue, QColor const& color);

  void send(QString const& cmd);

signals:
  void started();
  void initDone(QString const&);
  void generateDone(QString const&);

private slots:
  void processStarted();
  void processOutput();

private:
  QProcess* _process;
  iris::FifoReader _send;
  iris::FifoReader _reply;
  std::string _line;
  char _buffer[PIPE_BUF];
}; // class IrisProcess

#endif // HEV_HYDVIEW_IRISPROCESS_H

