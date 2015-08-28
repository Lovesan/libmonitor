#ifndef __LIBMONITOR_HPP__
#define __LIBMONITOR_HPP__

#include <exception>
#include <queue>
#include <ctime>
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

template<typename T, typename C = std::deque<T>>
class LmQueue
{
public:
    LmQueue() : _cap(-1) { }

    LmQueue(int cap) : _cap(cap) { }

    ~LmQueue() { }

    bool Add(const T& element, int timeout)
    {
        LmLock lock = _m.Lock();
        clock_t cl;
        if (_cap < 0)
        {
            _q.push(element);
            if (_q.size() == 1)
            {
                _m.PulseAll();
            }
            return true;
        }
        else
        {
            if (timeout < 0)
            {
                while (_q.size() >= _cap)
                {
                    _m.Wait();
                }
            }
            else
            {
                cl = clock();
                while (_q.size() >= _cap)
                {
                    if (timeout < 0 || !_m.Wait(timeout))
                    {
                        return false;
                    }
                    timeout -= ((clock() - cl) / CLOCKS_PER_SEC / 1000);
                    cl = clock();
                }
            }
            _q.push(element);
            if (_q.size() == 1)
            {
                _m.PulseAll();
            }
            return true;
        }
    }

    void Add(const T& element)
    {
        Add(element, -1);
    }

    bool Remove(T& element, int timeout)
    {
        LmLock lock = _m.Lock();
        clock_t cl;
        if (timeout < 0)
        {
            while (_q.empty())
            {
                _m.Wait();
            }
        }
        else
        {
            cl = clock();
            while (_q.empty())
            {
                if (timeout < 0 || !_m.Wait(timeout))
                {
                    return false;
                }
                timeout -= ((clock() - cl) / CLOCKS_PER_SEC / 1000);
                cl = clock();
            }
        }
        element = _q.front();
        _q.pop();
        if (_q.size() == (_cap - 1))
        {
            _m.PulseAll();
        }
        return true;
    }

    void Remove(T& element)
    {
        Remove(element, -1);
    }

    int GetSize()
    {
        Lock lock = _m.Lock();
        return _q.size();
    }

private:
    LmMonitor _m;
    std::queue<T, C> _q;
    int _cap;
};

#endif // __LIBMONITOR_HPP__