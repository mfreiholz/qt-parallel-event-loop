#ifndef _TCPOPENCONNECTIONBENCHMARK_HEADER_
#define _TCPOPENCONNECTIONBENCHMARK_HEADER_

#include "QtCore/QObject"
#include "QtCore/QString"
#include "QtCore/QTime"
#include "QtCore/QList"

struct Stats {
  int stat_packet_write_success_count;
  int stat_packet_write_failed_count;
  int stat_packet_read_success_count;
  int stat_packet_read_failed_count;
  int highest_response_time;
  int latest_response_time;

  Stats()
    : stat_packet_write_success_count(0),
      stat_packet_write_failed_count(0),
      stat_packet_read_success_count(0),
      stat_packet_read_failed_count(0),
      highest_response_time(0),
      latest_response_time(0)
  {
  }
};

class PersistentConnectionBenchmark
  : public QObject
{
  Q_OBJECT
public:
  PersistentConnectionBenchmark(int num, QObject *parent);
  ~PersistentConnectionBenchmark();
  void run();

private slots:
  void onConnectionFinished();

private:
  int _num_connections;
  QList<class ConnectionHandler*> _handlers;

  Stats _stats;
  QTime _run_timer;
};


class ConnectionHandler
  : public QObject
{
  Q_OBJECT
  friend class PersistentConnectionBenchmark;
public:
  ConnectionHandler(const QString &address, quint16 port, QObject *parent);
  ~ConnectionHandler();

public slots:
  void handle();
  void writeTestData();
  void stop();

private slots:
  void onConnected();
  void onDisconnected();
  void onReadyRead();

signals:
  void finished();

private:
  class QTcpSocket *_socket;
  QString _address;
  quint16 _port;

  int _interval_ms;
  int _repeat_count;

  QByteArray _packet;
  int _packet_size;
  QByteArray _inbuffer;

  Stats _stats;
  QTime _packet_cycle_time;
};

#endif
