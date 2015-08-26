#include <cstdlib>
#include <thread>
#include <iostream>
#include <queue>
#include "libmonitor.hpp"

void lm_test_thread_proc(LmMonitor* mon, std::queue<int> *q)
{
    for (int i = 1; i <= 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(int(rand() * 1.0f / RAND_MAX * 100 + 100)));
        mon->Enter();
        q->push(i);
        std::cout << "Pushed " << i << " to queue." << std::endl;
        mon->Pulse();
        mon->Exit();
    }
}

int main(void)
{
    LmMonitor mon;
    std::queue<int> q;
    std::thread t(lm_test_thread_proc, &mon, &q);
    try {
        for (int i = 1; i <= 10; ++i)
        {
            auto lock = mon.Lock();
            while (q.empty())
            {
                mon.Wait();
            }
            std::cout << "Got " << q.front() << " from queue." << std::endl;
            if (q.front() != i)
            {
                std::cout << "Abnormal value recieved" << std::endl;
                throw LmFailedException();
            }
            q.pop();
        }
        t.join();
        std::cout << "Test finished" << std::endl;
        return 0;
    }
    catch(...)
    {
        t.join();
        std::cout << "Test failed" << std::endl;
        return 1;
    }
}