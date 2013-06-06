#ifndef _PARALLELEVENTLOOP_HEADER_
#define _PARALLELEVENTLOOP_HEADER_

#include "QtCore/QObject"
#include "QtCore/QMutex"
#include "QtCore/QList"
#include "QtCore/QThread"

#include "apidef.h"

class ParallelEventLoopThread;


class QT_PEL_API ParallelEventLoopPool
  : public QObject
{
  Q_OBJECT
public:
  ParallelEventLoopPool(unsigned int numThreads, unsigned int numObjects, QObject *parent = 0);
  ~ParallelEventLoopPool();
  bool poolObject(QObject *obj);

protected:
  void timerEvent(class QTimerEvent *e);

private:
  void logStatistics() const;

private:
  mutable QMutex                  _lock;
  QList<ParallelEventLoopThread*> _threads;
  unsigned int                    _next_thread_index;
  unsigned int                    _max_objects_per_thread;
  int                             _log_statistic_timer_id;
};


class QT_PEL_API ParallelEventLoopThread
  : public QThread
{
  Q_OBJECT
public:
  ParallelEventLoopThread();
  int add(QObject *obj);
  void stop();
  unsigned int count() const;

protected:
  void run();

private slots:
  void onObjectDone();

private:
  mutable QMutex  _lock;
  bool            _stop;
  QList<QObject*> _objects;
};

#endif
