#include "libmonitor.hpp"

#define LM_DISPATCH_EXCEPTION(e,ptr,ms) \
            do{ \
                switch(e) \
                { \
                    case 0: break; \
                    case lm_timeout: throw LmTimeoutException(ms); break; \
                    case lm_pointer: throw LmPointerException(ptr); break; \
                    default: throw LmFailedException(); break; \
                } \
            } while(0)

LmException::LmException()
{
    _code = lm_ok;
}

LmException::LmException(lm_error code)
{
    _code = code;
}

LmException::~LmException()
{
}

lm_error LmException::GetCode()
{
    return _code;
}

LmFailedException::LmFailedException() : LmException(lm_failed)
{
}

LmTimeoutException::LmTimeoutException(int ms) : LmException(lm_timeout), _ms(ms)
{
}

int LmTimeoutException::GetTime()
{
    return _ms;
}

LmPointerException::LmPointerException(lm_monitor pointer) : LmException(lm_pointer), _pointer(pointer)
{
}

lm_monitor LmPointerException::GetPointer()
{
    return _pointer;
}

LmMonitor::LmMonitor()
{
    _m = (lm_monitor)(void*)0;
    lm_monitor m = lm_create();
    if (!m)
        throw LmFailedException();
    _m = m;
}

LmMonitor::~LmMonitor()
{
    if (_m)
    {
        lm_free(_m);
        _m = (lm_monitor)(void*)0;
    }
}

void LmMonitor::Enter()
{
    lm_error e = lm_enter(_m);
    LM_DISPATCH_EXCEPTION(e, _m, 0);
}

bool LmMonitor::TryEnter()
{
    lm_error e = lm_try_enter(_m);
    if (e == lm_failed)
        return false;
    LM_DISPATCH_EXCEPTION(e, _m, 0);
    return true;
}

void LmMonitor::Exit()
{
    lm_error e = lm_exit(_m);
    LM_DISPATCH_EXCEPTION(e, _m, 0);
}

bool LmMonitor::Wait(int ms)
{
    lm_error e = lm_wait(_m, ms);
    if (e == lm_timeout)
        return false;
    LM_DISPATCH_EXCEPTION(e, _m, ms);
    return true;
}

void LmMonitor::Wait()
{
    lm_error e = lm_wait(_m, -1);
    LM_DISPATCH_EXCEPTION(e, _m, -1);
}

void LmMonitor::Pulse()
{
    lm_error e = lm_pulse(_m);
    LM_DISPATCH_EXCEPTION(e, _m, 0);
}

void LmMonitor::PulseAll()
{
    lm_error e = lm_pulse_all(_m);
    LM_DISPATCH_EXCEPTION(e, _m, 0);
}

LmMonitor& LmMonitor::operator=(const LmMonitor & rhs)
{
    if (&rhs == this)
        return *this;
    if (_m)
    {
        lm_free(_m);
        _m = (lm_monitor)(void*)0;
    }
    return *this;
}

LmLock LmMonitor::Lock()
{
    return LmLock(_m);
}

LmMonitor::LmMonitor(const LmMonitor & rhs)
{
    _m = (lm_monitor)(void*)0;
}

LmLock::LmLock(lm_monitor monitor)
{
    _m = monitor;
}

LmLock::LmLock(LmLock& rhs)
{
    _m = rhs._m;
    rhs._m = (lm_monitor)(void*)0;
}

LmLock::~LmLock()
{
    if (_m)
    {
        lm_exit(_m);
        _m = (lm_monitor)(void*)0;
    }
}

LmLock& LmLock::operator=(LmLock& rhs)
{
    if (&rhs == this)
        return *this;
    Exit();
    _m = rhs._m;
    rhs._m = (lm_monitor)(void*)0;
    return *this;
}

void LmLock::Exit()
{
    lm_error e;
    if (_m)
    {
        e = lm_exit(_m);
        LM_DISPATCH_EXCEPTION(e, _m, -1);
        _m = (lm_monitor)(void*)0;
    }
}