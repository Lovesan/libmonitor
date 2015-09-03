#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "libmonitor.h"

struct _lm_monitor_struct
{
  int ready;
  pthread_cond_t cv;
  pthread_mutexattr_t a;
  pthread_mutex_t m;
};

lm_monitor lm_create()
{
  lm_monitor m = (lm_monitor)calloc(1, sizeof(struct _lm_monitor_struct));
  if(!m)
    return m;

  if(pthread_mutexattr_init(&m->a))
  {
    free(m);
    return 0;
  }

  if(pthread_mutexattr_settype(&m->a, PTHREAD_MUTEX_RECURSIVE))
  {
    pthread_mutexattr_destroy(&m->a);
    free(m);
    return 0;
  }

  if(pthread_mutex_init(&m->m, &m->a))
  {
    pthread_mutexattr_destroy(&m->a);
    free(m);
    return 0;
  }

  if(pthread_cond_init(&m->cv, NULL))
  {
    pthread_mutexattr_destroy(&m->a);
    pthread_mutex_destroy(&m->m);
    free(m);
    return 0;
  }

  m->ready = 1;

  return m;
}

#define LM_VALID(m) ((m) && (m->ready))

lm_error lm_is_valid(lm_monitor monitor)
{
  return LM_VALID(monitor) ? lm_ok : lm_failed;
}

lm_error lm_free(lm_monitor monitor)
{
  if(!LM_VALID(monitor))
    return lm_pointer;

  if(pthread_mutexattr_destroy(&monitor->a))
    return lm_failed;
  if(pthread_mutex_destroy(&monitor->m))
    return lm_failed;
  if(pthread_cond_destroy(&monitor->cv))
    return lm_failed;
  monitor->ready = 0;
  free(monitor);
  return lm_ok;
}

lm_error lm_enter(lm_monitor monitor)
{
  if(!LM_VALID(monitor))
    return lm_pointer;

  if(pthread_mutex_lock(&monitor->m))
    return lm_failed;
    
  return lm_ok;
}

lm_error lm_try_enter(lm_monitor monitor)
{
  if(!LM_VALID(monitor))
    return lm_pointer;

  if(pthread_mutex_trylock(&monitor->m))
    return lm_failed;

  return lm_ok;
}

lm_error lm_exit(lm_monitor monitor)
{
  if(!LM_VALID(monitor))
    return lm_pointer;

  if(pthread_mutex_unlock(&monitor->m))
    return lm_failed;

  return lm_ok;
}

lm_error lm_wait(lm_monitor monitor, int ms)
{
  int err;
  struct timespec ts;

  if(!LM_VALID(monitor))
    return lm_pointer;

  ts.tv_sec = (ms / 1000);
  ts.tv_nsec = (ms % 1000) * 1000000;

  if(ms < 0)
  {
    if(pthread_cond_wait(&monitor->cv, &monitor->m))
      return lm_failed;
  }
  else
  {
    err = pthread_cond_timedwait(&monitor->cv, &monitor->m, &ts);
    if(err)
      return err == ETIMEDOUT ? lm_timeout : lm_failed;
  }

  return lm_ok;
}

lm_error lm_pulse(lm_monitor monitor)
{
  if(!LM_VALID(monitor))
    return lm_pointer;

  if(pthread_cond_signal(&monitor->cv))
    return lm_failed;

  return lm_ok;
}

lm_error lm_pulse_all(lm_monitor monitor)
{
  if(!LM_VALID(monitor))
    return lm_pointer;

  if(pthread_cond_broadcast(&monitor->cv))
    return lm_failed;

  return lm_ok;
}
