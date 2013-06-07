#include "paralleleventloop.h"

#include <cstdio>

#include "QtCore/QTimerEvent"

///////////////////////////////////////////////////////////////////////////////
// ParallelEventLoopPool
///////////////////////////////////////////////////////////////////////////////

ParallelEventLoopPool::ParallelEventLoopPool(unsigned int numThreads, unsigned int numObjects, QObject *parent)
  : QObject(parent),
    _lock(),
    _threads(),
    _next_thread_index(0),
    _max_objects_per_thread(numObjects),
    _log_statistic_timer_id(-1)
{
  for (unsigned int i = 0; i < numThreads; ++i) {
    ParallelEventLoopThread *t = new ParallelEventLoopThread();
    _threads.append(t);
    t->start();
  }
  _log_statistic_timer_id = startTimer(2000);
}

ParallelEventLoopPool::~ParallelEventLoopPool()
{
  for (int i = 0; i < _threads.count(); ++i) {
    _threads[i]->quit();
    _threads[i]->wait();
    delete _threads[i];
  }
  _threads.clear();

  if (_log_statistic_timer_id >= 0)
    killTimer(_log_statistic_timer_id);
}

bool ParallelEventLoopPool::poolObject(QObject *obj)
{
  bool moved = false;
  _lock.lock();
  for (int i = 0; i < _threads.size(); ++i) {
    if (++_next_thread_index > (unsigned int)_threads.size()-1)
      _next_thread_index = 0;

    ParallelEventLoopThread *t = _threads.at(_next_thread_index);
    if (t->count() < _max_objects_per_thread) {
      t->add(obj);
      moved = true;
      break;
    }
  }
  _lock.unlock();

  if (!moved)
    fprintf(stdout, "No slot left in worker threads. Keep it in main event loop.\n");
  return moved;
}

void ParallelEventLoopPool::timerEvent(class QTimerEvent *e)
{
  if (e->timerId() == _log_statistic_timer_id)
    logStatistics();
}

void ParallelEventLoopPool::logStatistics() const
{
  _lock.lock();
  for (int i = 0; i < _threads.count(); ++i) {
    fprintf(stdout, "Thread #%d: %d QObjects\n", i, _threads[i]->count());
  }
  _lock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
// ParallelEventLoopThread
///////////////////////////////////////////////////////////////////////////////

ParallelEventLoopThread::ParallelEventLoopThread()
  : QThread(),
    _stop(false)
{
}

int ParallelEventLoopThread::add(QObject *obj)
{
  _lock.lock();
  _objects.append(obj);
  obj->moveToThread(this);
  connect(obj, SIGNAL(done()), SLOT(onObjectDone()));
  int c = _objects.count();
  _lock.unlock();
  return c;
}

void ParallelEventLoopThread::stop()
{
  _lock.lock();
  _stop = true;
  QList<QObject*> objects = _objects;
  _lock.unlock();

  for (int i = 0; i < objects.size(); ++i) {
    QMetaObject::invokeMethod(objects[i], "stop");
  }
}

unsigned int ParallelEventLoopThread::count() const
{
  _lock.lock();
  int i = _objects.count();
  _lock.unlock();
  return i;
}

void ParallelEventLoopThread::run()
{
  exec();

  _lock.lock();
  for (int i = 0; i < _objects.size(); ++i) {
    delete _objects[i];
  }
  _objects.clear();
  _lock.unlock();
}

void ParallelEventLoopThread::onObjectDone()
{
  QObject *obj = sender();
  _lock.lock();
  _objects.removeAll(obj);
  bool call_quit = (_stop && _objects.size() == 0);
  _lock.unlock();
  obj->deleteLater();
  if (call_quit)
    quit();
}
