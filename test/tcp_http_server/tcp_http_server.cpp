#include "tcp_http_server.h"
#include "QtCore/QDir"
#include "QtCore/QFileInfo"
#include "QtCore/QFile"
#include "QtCore/QByteArray"
#include "QtCore/QString"
#include "QtCore/QEventLoop"
#include "QtNetwork/QTcpSocket"

///////////////////////////////////////////////////////////////////////////////
// TcpServer
///////////////////////////////////////////////////////////////////////////////

TcpServer::TcpServer(unsigned int numThreads)
  : QTcpServer(),
    _eventLoopPool(numThreads, 1)
{
  // Create test files.
  QString file_512bytes_path = QDir::tempPath() + "/512B.bin";
  QFileInfo fi(file_512bytes_path);
  if (!fi.exists()) {
    QByteArray arr;
    while (arr.size() < 512)
      arr.append("ab");
    QFile f(fi.filePath());
    f.open(QIODevice::WriteOnly);
    f.write(arr);
    f.close();
  }
}

TcpServer::~TcpServer()
{
  QFileInfo fi(QDir::tempPath() + "/512B.bin");
  if (fi.exists())
    QFile(fi.filePath()).remove();
}

void TcpServer::incomingConnection(int socketDescriptor)
{
  // Use ParallelEventLoopPool.
  PersistentEchoRequestHandler *req = new PersistentEchoRequestHandler(socketDescriptor);
  if (!_eventLoopPool.poolObject(req)) {
    // No free worker thread available.
    // PersistentEchoRequestHandler stays in Main-Thread.
  }
  QMetaObject::invokeMethod(req, "handleRequest", Qt::QueuedConnection);
}

///////////////////////////////////////////////////////////////////////////////
// PersistentEchoRequestHandler
///////////////////////////////////////////////////////////////////////////////

PersistentEchoRequestHandler::PersistentEchoRequestHandler(int socketDescriptor)
  : QObject(0),
    _descriptor(socketDescriptor),
    _socket(0)
{
}

PersistentEchoRequestHandler::~PersistentEchoRequestHandler()
{
}

void PersistentEchoRequestHandler::handleRequest()
{
  _socket = new QTcpSocket(this);
  connect(_socket, SIGNAL(disconnected()), SIGNAL(done()));
  connect(_socket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
  _socket->setSocketDescriptor(_descriptor);
}

void PersistentEchoRequestHandler::onSocketReadyRead()
{
  QByteArray data = _socket->readAll();
  _socket->write(data);
}

///////////////////////////////////////////////////////////////////////////////
// HttpRequestHandler
///////////////////////////////////////////////////////////////////////////////

HttpRequestHandler::HttpRequestHandler(int socketDescriptor)
  : QObject(0),
    _descriptor(socketDescriptor),
    _socket(0)
{
}

HttpRequestHandler::~HttpRequestHandler()
{
}

void HttpRequestHandler::handleRequest()
{
  _socket = new QTcpSocket(this);
  connect(_socket, SIGNAL(disconnected()), SIGNAL(done()));
  connect(_socket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
  _socket->setSocketDescriptor(_descriptor);
}

void HttpRequestHandler::onSocketReadyRead()
{
  // Read incoming request.
  QByteArray data = _socket->readAll();

  // Read 512B file.
  QFileInfo fi(QDir::tempPath() + "/512B.bin");
  QFile f(fi.filePath());
  if (!f.open(QIODevice::ReadOnly)) {
    _socket->close();
    _socket->deleteLater();
  }
  QByteArray file_content = f.readAll();
  f.close();

  // Simulate some work.
  //QThread::msleep(5);

  // Write response.
  int thread_id = (int)QThread::currentThreadId();
  _socket->write(QString(
    "HTTP/1.1 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: %1\r\n"
    "\r\n%2").arg(file_content.size()).arg(file_content.data()).toStdString().c_str());
  _socket->flush();
  _socket->close();
  _socket->deleteLater();
}

///////////////////////////////////////////////////////////////////////////////
// ParallelEventLoopTask
///////////////////////////////////////////////////////////////////////////////

RequestHandlerTask::RequestHandlerTask(int socketDescriptor)
: QObject(0),
  QRunnable(),
  _descriptor(socketDescriptor)
{
}

void RequestHandlerTask::run()
{
  QEventLoop *el = new QEventLoop(NULL);

  PersistentEchoRequestHandler *req = new PersistentEchoRequestHandler(_descriptor);
  connect(req, SIGNAL(done()), el, SLOT(quit()));
  req->handleRequest();

  el->exec();
  el->processEvents();
  delete el;
  delete req;
}

///////////////////////////////////////////////////////////////////////////////
// Save some code below.
///////////////////////////////////////////////////////////////////////////////

// Do everything in main thread.
//RequestHandler *req = new RequestHandler(socketDescriptor);
//connect(req, SIGNAL(done()), req, SLOT(deleteLater()));
//req->handleRequest();

// Use QThreadPool with QRunnable objects.
//RequestHandlerTask *req = new RequestHandlerTask(socketDescriptor);
//_taskPool.start(req);
