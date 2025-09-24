//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#pragma once

#include "Types.hpp"
#include <pthread.h>
#include <mutex>
#include <thread>
#include <unordered_map>

#if defined(ANDROID) || defined(__ANDROID__)
#include <android/looper.h>
#include <unistd.h>
#endif

#define THREAD_STR	"["<<get_thread_id()<<"]"

extern std::size_t get_thread_id() noexcept;

class Thread
{
    bool mOnLooper;
    pthread_t mThreadId;
    std::thread mThread;
	sched_param sch_params;

#if defined(ANDROID) || defined(__ANDROID__)
    ALooper* mThreadLooper;
    int mPipe[2];
#endif

public:
	Thread(bool bOnLooper = false);
	virtual ~Thread();

	virtual void start();
	virtual void join();
	virtual void setScheduling(int priority);
	virtual void run(void*) = 0;

    virtual void send(char);
    virtual void rcv(char);

private:
	static void* threadStartCallback(void*);
    static int looperCallback(int, int, void*);
};

