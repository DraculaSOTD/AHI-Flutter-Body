//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AutoLock.hpp"

AutoLock::AutoLock(Mutex& lock) : mLock(lock)
{
	mLock.lock();
}

AutoLock::~AutoLock()
{
	mLock.unlock();
}

void AutoLock::lock()
{
	mLock.lock();
}

void AutoLock::unlock()
{
	mLock.unlock();
}
