#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

#define THREAD_MAX (3)
#define END (10)

mutex mtx;
condition_variable cv;
int cid = 0;
int current = 0;

void print_func(int id)
{
    unique_lock<mutex> lck(mtx);
    while (true)
    {
        while(cid != id && current < END)
            cv.wait(lck);
        if (current == END)
            break;
        cout << "Thread " << id << ": " << current << endl;
        cid = (cid + 1) % THREAD_MAX;
        current += !cid;
        cv.notify_all();
    }
}

void go() {
  unique_lock<mutex> lck(mtx);
  cv.notify_all();
}

int main()
{
    thread threads[THREAD_MAX];
    for (int i = 0; i < THREAD_MAX; ++i)
        threads[i] = thread(print_func, i);
    go();
    for (auto &th : threads) th.join();
    return 0;
}
