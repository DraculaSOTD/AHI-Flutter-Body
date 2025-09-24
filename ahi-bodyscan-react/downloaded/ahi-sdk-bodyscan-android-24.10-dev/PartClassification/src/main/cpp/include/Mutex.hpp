//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#pragma once

#include "Types.hpp"
#include <pthread.h>

class Mutex
{
	pthread_mutex_t* mMutex;
	bool mCopy;

public:
	Mutex();
	Mutex(Mutex&);
	virtual ~Mutex();

	void lock();
	void unlock();
	pthread_mutex_t* get();
};

