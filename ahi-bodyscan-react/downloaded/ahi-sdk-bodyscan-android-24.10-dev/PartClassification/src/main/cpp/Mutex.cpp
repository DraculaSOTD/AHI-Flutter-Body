//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "include/Mutex.hpp"


Mutex::Mutex() : mMutex(new pthread_mutex_t), mCopy(false)
{
	pthread_mutexattr_t Attr;

	pthread_mutexattr_init(&Attr);
	pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(mMutex, &Attr);
}

Mutex::Mutex(Mutex& copy) : mMutex(copy.mMutex), mCopy(true)
{

}

Mutex::~Mutex()
{
	if (!mCopy)
	{
		pthread_mutex_destroy(mMutex);
		delete mMutex;
	}
}

void Mutex::lock()
{
	pthread_mutex_lock(mMutex);
}

void Mutex::unlock()
{
	pthread_mutex_unlock(mMutex);
}

pthread_mutex_t* Mutex::get()
{
	return mMutex;
}
