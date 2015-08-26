#ifndef __LIBMONITOR_HPP__
#define __LIBMONITOR_HPP__

#include <exception>
#include "libmonitor.h"

#ifdef _WIN32
#ifdef LM_BUILD_DLL
#define LM_EXTERN_CLASS __declspec(dllexport)
#else
#define LM_EXTERN_CLASS __declspec(dllimport)
#endif
#endif

class LM_EXTERN_CLASS LmException
{
public:
    virtual ~LmException();

    lm_error GetCode();

protected:
    LmException(lm_error code);

private:
    lm_error _code;

    LmException();
};

class LM_EXTERN_CLASS LmFailedException : public LmException
{
public:
    LmFailedException();
};

class LM_EXTERN_CLASS LmTimeoutException : public LmException
{
public:
    LmTimeoutException(int ms);

    int GetTime();

private:
    int _ms;
};

class LM_EXTERN_CLASS LmPointerException : public LmException
{
public:
    LmPointerException(lm_monitor pointer);

    lm_monitor GetPointer();

private:
    lm_monitor _pointer;
};

class LM_EXTERN_CLASS LmLock
{
public:
    LmLock(lm_monitor monitor);
    LmLock(LmLock& rhs);
    LmLock& operator=(LmLock& rhs);
    ~LmLock();
    void Exit();
private:
    lm_monitor _m;
    LmLock();
};

class LM_EXTERN_CLASS LmMonitor
{
public:    
    LmMonitor();
    
    ~LmMonitor();

    void Enter();
    bool TryEnter();
    LmLock Lock();
    void Exit();
    bool Wait(int ms);
    void Wait();
    void Pulse();
    void PulseAll();

private:
    lm_monitor _m;
    
    LmMonitor(const LmMonitor& rhs);
    LmMonitor& operator=(const LmMonitor& rhs);
};

#endif // __LIBMONITOR_HPP__