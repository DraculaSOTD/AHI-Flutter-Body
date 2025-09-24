//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#pragma once

#include "Types.hpp"
#include "Mutex.hpp"

class AutoLock
{
	Mutex mLock;

public:
	AutoLock(Mutex& lock);
	virtual ~AutoLock();
	void lock();
	void unlock();
};
