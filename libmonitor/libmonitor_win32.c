#include <windows.h>
#include "libmonitor.h"

struct _lm_monitor_struct {
    BOOL ready;
    CRITICAL_SECTION cs;
    CONDITION_VARIABLE cv;
};

lm_monitor lm_create()
{
    lm_monitor m = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct _lm_monitor_struct));
    
    if (!m)
        return m;

    InitializeCriticalSection(&m->cs);
    InitializeConditionVariable(&m->cv);

    m->ready = TRUE;

    return m;
}

lm_error lm_is_valid(lm_monitor monitor)
{
    if (!monitor || !monitor->ready)
        return lm_pointer;
    return lm_ok;
}

lm_error lm_free(lm_monitor monitor)
{
    if (!monitor || !monitor->ready)
        return lm_pointer;
    monitor->ready = FALSE;
    DeleteCriticalSection(&monitor->cs);
    HeapFree(GetProcessHeap(), 0, monitor);
    return lm_ok;
}

lm_error lm_enter(lm_monitor monitor)
{
    if (!monitor || !monitor->ready)
        return lm_pointer;
    EnterCriticalSection(&monitor->cs);
    return lm_ok;
}

lm_error lm_try_enter(lm_monitor monitor)
{
    if (!monitor || !monitor->ready)
        return lm_pointer;
    if (TryEnterCriticalSection(&monitor->cs))
        return lm_ok;
    return lm_failed;
}

lm_error lm_exit(lm_monitor monitor)
{
    if (!monitor || !monitor->ready)
        return lm_pointer;
    LeaveCriticalSection(&monitor->cs);
    return lm_ok;
}

lm_error lm_wait(lm_monitor monitor, int ms)
{
    DWORD timeout = (ms >= 0) ? ((DWORD)ms) : INFINITE;
    
    if (!monitor || !monitor->ready)
        return lm_pointer;

    if (SleepConditionVariableCS(&monitor->cv, &monitor->cs, timeout))
        return lm_ok;
    return (GetLastError() == ERROR_TIMEOUT) ? lm_timeout : lm_pointer;
}

lm_error lm_pulse(lm_monitor monitor)
{
    if (!monitor || !monitor->ready)
        return lm_pointer;
    WakeConditionVariable(&monitor->cv);
    return lm_ok;
}

lm_error lm_pulse_all(lm_monitor monitor)
{
    if (!monitor || !monitor->ready)
        return lm_pointer;
    WakeAllConditionVariable(&monitor->cv);
    return lm_ok;
}