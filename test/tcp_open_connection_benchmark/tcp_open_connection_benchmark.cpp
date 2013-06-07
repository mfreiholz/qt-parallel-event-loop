#include "tcp_open_connection_benchmark.h"

#include "QtCore/QTimer"
#include "QtCore/QThread"
#include "QtNetwork/QTcpSocket"

///////////////////////////////////////////////////////////////////////////////
// PersistentConnectionBenchmark
///////////////////////////////////////////////////////////////////////////////

PersistentConnectionBenchmark::PersistentConnectionBenchmark(int numConnections, int numPacketsPerConnection, QObject *parent)
  : QObject(parent),
    _num_connections(numConnections),
    _num_packets_per_connection(numPacketsPerConnection),
    _handlers()
{
}

PersistentConnectionBenchmark::~PersistentConnectionBenchmark()
{
}

void PersistentConnectionBenchmark::run()
{
  _run_timer.start();
  for (int i = 0; i < _num_connections; ++i) {
    // Single threaded.
//    ConnectionHandler *h = new ConnectionHandler("127.0.0.1", 1337, _num_packets_per_connection, 0);
//    _handlers.append(h);
//    connect(h, SIGNAL(finished()), SLOT(onConnectionFinished()));
//    h->handle();

    // Multi threaded (Think about a PEL here!)
    ConnectionHandler *h = new ConnectionHandler("127.0.0.1", 1337, _num_packets_per_connection, 0);
    _handlers.append(h);

    QThread *t = new QThread(this);
    h->moveToThread(t);
    connect(t, SIGNAL(started()), h, SLOT(handle()));
    connect(h, SIGNAL(finished()), SLOT(onConnectionFinished()));
    connect(h, SIGNAL(finished()), t, SLOT(quit()));
    t->start();
  }
}

void PersistentConnectionBenchmark::onConnectionFinished()
{
  ConnectionHandler *h = qobject_cast<ConnectionHandler*>(sender());

  // Update global stats.
  _stats.stat_packet_write_success_count += h->_stats.stat_packet_write_success_count;
  _stats.stat_packet_write_failed_count  += h->_stats.stat_packet_write_failed_count;

  _stats.stat_packet_read_success_count  += h->_stats.stat_packet_read_success_count;
  _stats.stat_packet_read_failed_count   += h->_stats.stat_packet_read_failed_count;

  if (h->_stats.highest_response_time > _stats.highest_response_time)
    _stats.highest_response_time = h->_stats.highest_response_time;

  _handlers.removeAll(h);
  if (_handlers.isEmpty()) {
    int runtime_ms = _run_timer.elapsed();

    fprintf(stdout,
        "\n"
        "---------------\n"
        "Final statistic\n"
        "---------------\n"
        "Packets write success       : %d\n"
        "Packets write failed        : %d\n"
        "Packets read success        : %d\n"
        "\n"
        "Highest response time       : %d ms\n"
        "\n"
        "Runtime                     : %d ms\n"
        "\n",
        _stats.stat_packet_write_success_count,
        _stats.stat_packet_write_failed_count,
        _stats.stat_packet_read_success_count,
        _stats.highest_response_time,
        runtime_ms
    );
  }
}

///////////////////////////////////////////////////////////////////////////////
// ConnectionHandler
///////////////////////////////////////////////////////////////////////////////

ConnectionHandler::ConnectionHandler(const QString &address, quint16 port, int numPackets, QObject *parent)
  : QObject(parent),
    _socket(0),
    _address(address),
    _port(port),
    _interval_ms(0),
    _repeat_count(numPackets),
    _packet_size(512),
    _inbuffer()
{
  // Create test packet, which will be send to the server.
  while (_packet.size() < _packet_size)
    _packet.append("x");

  // Prepare input buffer.
  _inbuffer.reserve(_packet_size);

  _stats.stat_packet_read_success_count = 0;
  _stats.stat_packet_read_failed_count = 0;
  _stats.stat_packet_write_success_count = 0;
  _stats.stat_packet_write_failed_count = 0;
  _stats.highest_response_time = 0;
  _stats.latest_response_time = 0;
}

ConnectionHandler::~ConnectionHandler()
{
}

void ConnectionHandler::handle()
{
  _socket = new QTcpSocket(this);
  connect(_socket, SIGNAL(connected()), SLOT(onConnected()));
  connect(_socket, SIGNAL(disconnected()), SLOT(onDisconnected()));
  connect(_socket, SIGNAL(readyRead()), SLOT(onReadyRead()));
  _socket->connectToHost(_address, _port);
}

void ConnectionHandler::writeTestData()
{
  _packet_cycle_time.restart();

  if (_socket->write(_packet) != _packet_size) {
    ++_stats.stat_packet_write_failed_count;
  } else {
    ++_stats.stat_packet_write_success_count;
  }
}

void ConnectionHandler::stop()
{
  _socket->disconnectFromHost();
}

void ConnectionHandler::onConnected()
{
  QTimer::singleShot(_interval_ms, this, SLOT(writeTestData()));
}

void ConnectionHandler::onDisconnected()
{
  emit finished();
}

void ConnectionHandler::onReadyRead()
{
  if (_inbuffer.size() < _packet_size) {
    QByteArray ba = _socket->read(_packet_size - _inbuffer.size());
    _inbuffer.append(ba);
  }

  if (_inbuffer.size() < _packet_size)
    return;

  // Complete.
  int response_time_ms = _packet_cycle_time.elapsed();

  ++_stats.stat_packet_read_success_count;

  _stats.latest_response_time = response_time_ms;
  if (_stats.highest_response_time < _stats.latest_response_time)
    _stats.highest_response_time = _stats.latest_response_time;

  _inbuffer.clear();

  if ((_stats.stat_packet_write_success_count + _stats.stat_packet_write_failed_count) < _repeat_count)
    QTimer::singleShot(_interval_ms, this, SLOT(writeTestData()));
  else
    QTimer::singleShot(_interval_ms, this, SLOT(stop()));
}
