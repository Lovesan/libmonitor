#include <cstdlib>
#include <thread>
#include <iostream>
#include "libmonitor.hpp"

void lm_test_thread_proc(LmQueue<int> *q)
{
    for (int i = 1; i <= 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(int(rand() * 1.0f / RAND_MAX * 100 + 100)));
        q->Add(i);
    }
}

int main(void)
{
    LmQueue<int> q(3);
    int x;
    std::thread t(lm_test_thread_proc, &q);
    try {
        for (int i = 1; i <= 10; ++i)
        {
            q.Remove(x);
            if (x != i)
            {
                std::cout << "Abnormal value recieved" << std::endl;
                throw LmFailedException();
            }
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