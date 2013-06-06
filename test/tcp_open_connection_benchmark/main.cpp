#include "QtCore/QCoreApplication"
#include "QtCore/QStringList"

#include "tcp_open_connection_benchmark.h"

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  // Parameter values.
  int num_connections = 0;
  int num_packets_per_connection = 0;

  int pos = -1;
  if ((pos = app.arguments().indexOf("-n")) != -1) {
    num_connections = app.arguments().value(pos + 1, "0").toInt();
  }
  if ((pos = app.arguments().indexOf("-p")) != -1) {
    num_packets_per_connection = app.arguments().value(pos + 1, "0").toInt();
  }

  if (num_connections <= 0)
    return 0;

  PersistentConnectionBenchmark bench(num_connections, num_packets_per_connection, 0);
  bench.run();

  return app.exec();
}
