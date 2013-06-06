#include "QtCore/QCoreApplication"
#include "QtCore/QStringList"

#include "tcp_open_connection_benchmark.h"

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  // Parameter values.
  int num_connections = 0;

  int pos = -1;
  if ((pos = app.arguments().indexOf("-n")) != -1) {
    num_connections = app.arguments().value(pos + 1, "0").toInt();
  }

  if (num_connections <= 0)
    return 0;

  PersistentConnectionBenchmark bench(num_connections, 0);
  bench.run();

  return app.exec();
}
