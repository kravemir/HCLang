/*
 * HCLang - Highly Concurrent Language
 * Copyright (c) 2015 Miroslav Kravec
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "base.hpp"

#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>

#include <unistd.h>
#include <corelib/async_io.h>

// Multi Threaded executor implementation

template <typename T>
class blockqueue
{
private:
    std::mutex              d_mutex;
    std::condition_variable d_condition;
    std::deque<T>           d_queue;
    int                     d_workersOnline = 0;
public:
    void push(T const& value) {
        {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            d_queue.push_front(value);
        }
        this->d_condition.notify_one();
    }
    T donePop() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        d_workersOnline -= 1;
        if(d_workersOnline == 0 && d_queue.empty()) {
            this->d_condition.notify_all();
            return 0;
        }
        this->d_condition.wait(lock, [=]{
            return !this->d_queue.empty() || d_workersOnline == 0;
        });
        if( !d_queue.empty() ) {
            T rc(std::move(this->d_queue.back()));
            this->d_queue.pop_back();
            d_workersOnline += 1;
            return rc;
        } else {
            return 0;
        }
    }
    T pop() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        this->d_condition.wait(lock, [=]{
            return !this->d_queue.empty() || d_workersOnline == 0;
        });
        if( !d_queue.empty() ) {
            T rc(std::move(this->d_queue.back()));
            this->d_queue.pop_back();
            d_workersOnline += 1;
            return rc;
        } else {
            return 0;
        }
    }
};

struct MTExecutor : Executor {
    blockqueue<System*> queue;
};

static
void mtexecutor_threadloop(MTExecutor *e) {
    System *s = e->queue.pop();

RESTART:
    while(s) {
        // current work is too short to utilize multiple threads,
        // debug it with usleep to see it working multithreaded
        usleep(1000);
        system_executeWork(s);

        s = e->queue.donePop();
    }

    // TODO: fix active waiting for async io
    if(asyncIoHasPendingReads()) {
        s = e->queue.pop();
        goto RESTART;
    }
}

static
void mtexecutor_mainloop(MTExecutor *e) {
    std::thread t1(mtexecutor_threadloop, e);
    std::thread t2(mtexecutor_threadloop, e);

    t1.join();
    t2.join();
}

static
void mtexecutor_putWork(MTExecutor *e, System *s) {
    e->queue.push(s);
}

extern "C" {
Executor* mtexecutor_new() {
    MTExecutor *e = (MTExecutor*)malloc(sizeof(MTExecutor));
    new (e) MTExecutor();
    e->run = (void(*)(Executor*))mtexecutor_mainloop;
    e->putWork = (void(*)(Executor*,System*))mtexecutor_putWork;
    return e;
}
}

