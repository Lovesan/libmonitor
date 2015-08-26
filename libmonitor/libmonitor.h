#ifndef __LIBMONITOR_H__
#define __LIBMONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct _lm_monitor_struct;

typedef struct _lm_monitor_struct *lm_monitor;

enum _lm_error
{
    lm_ok = 0,
    lm_failed = 1,
    lm_timeout = 2,
    lm_pointer = 3
};

typedef enum _lm_error lm_error;

lm_monitor lm_create();

lm_error lm_is_valid(lm_monitor monitor);

lm_error lm_free(lm_monitor monitor);

lm_error lm_enter(lm_monitor monitor);

lm_error lm_try_enter(lm_monitor monitor);

lm_error lm_exit(lm_monitor monitor);

lm_error lm_wait(lm_monitor monitor, int ms);

lm_error lm_pulse(lm_monitor monitor);

lm_error lm_pulse_all(lm_monitor monitor);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LIBMONITOR_H__