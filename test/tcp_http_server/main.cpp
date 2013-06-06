#include "QtCore/QCoreApplication"
#include "QtCore/QThread"
#include "tcp_http_server.h"

int main(int argc, char **args)
{
  QCoreApplication qapp(argc, args);

  TcpServer server(QThread::idealThreadCount(), 50);
  server.listen(QHostAddress::Any, 1337);

  return qapp.exec();
}
