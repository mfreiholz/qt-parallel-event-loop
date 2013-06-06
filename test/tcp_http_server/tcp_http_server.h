#ifndef _TEST_TCP_HTTP_SERVER_HEADER_
#define _TEST_TCP_HTTP_SERVER_HEADER_

#include "QtCore/QObject"
#include "QtCore/QThreadPool"
#include "QtCore/QRunnable"
#include "QtNetwork/QTcpServer"
#include "paralleleventloop.h"
class QTcpSocket;


class TcpServer
  : public QTcpServer
{
public:
  TcpServer(unsigned int numThreads);
  ~TcpServer();

protected:
  virtual void incomingConnection(int socketDescriptor);

private:
  ParallelEventLoopPool _eventLoopPool;
};


class PersistentEchoRequestHandler
  : public QObject
{
  Q_OBJECT
public:
  PersistentEchoRequestHandler(int socketDescriptor);
  ~PersistentEchoRequestHandler();

public slots:
  void handleRequest();

private slots:
  void onSocketReadyRead();

signals:
  void done();

private:
  int _descriptor;
  QTcpSocket *_socket;
};


class HttpRequestHandler
  : public QObject
{
  Q_OBJECT
public:
  HttpRequestHandler(int socketDescriptor);
  ~HttpRequestHandler();

public slots:
  void handleRequest();

private slots:
  void onSocketReadyRead();

signals:
      void done();

private:
  int _descriptor;
  QTcpSocket *_socket;
};


class RequestHandlerTask
  : public QObject,
    public QRunnable
{
  Q_OBJECT
public:
  RequestHandlerTask(int socketDescriptor);
  void run();

signals:
  void done();

private:
  int _descriptor;
};

#endif
