#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>


template <class T, class _Container = vector<T>, class _Compare = less<typename _Container::value_type>>
class PriorityQueue {
public:
	// Data
    std::priority_queue<T, _Container, _Compare> mQueue;
	//std::priority_queue<T, vector<T>, AVCompare<T>> mQueue;
	typename std::priority_queue<T>::size_type mMaxQueueSize;

	// Thread gubbins
	std::mutex mMutex;
	std::condition_variable mFull;
	std::condition_variable mEmpty;

	// Exit
	std::atomic_bool mQuit{false};
	std::atomic_bool mFinished{false};

	PriorityQueue(const size_t size_max);

	bool push(T &&data);
	bool pop(T &data);
	bool popWait(T &data);
    bool popNoWait(T &data, bool &gotFrame);

    void notifyFullWaiting();
    void notifyEmptyWaiting();

    void flush();
    void reset();

    void lock();
    void unlock();

	// The PriorityQueue has finished accepting input
	void finished();
	// The PriorityQueue will cannot be pushed or popped
	void quit();

};

template <class T, class _Container, class _Compare>
PriorityQueue<T, _Container, _Compare>::PriorityQueue(size_t size_max) :
		mMaxQueueSize{size_max} {
}

template <class T, class _Container, class _Compare>
void PriorityQueue<T, _Container, _Compare>::notifyFullWaiting()
{
    mFull.notify_all();
}

template <class T, class _Container, class _Compare>
void PriorityQueue<T, _Container, _Compare>::notifyEmptyWaiting()
{
    mEmpty.notify_all();
}

template <class T, class _Container, class _Compare>
bool PriorityQueue<T, _Container, _Compare>::push(T &&data)
{
	std::unique_lock<std::mutex> lock(mMutex);

	while (!mQuit && !mFinished)
	{
		if ((0 == mMaxQueueSize) || (mQueue.size() <= mMaxQueueSize))
		{
			mQueue.push(std::move(data));

			mEmpty.notify_all();
			return true;
		}
		else
		{
			mFull.wait(lock);
		}
	}

	return false;
}

template <class T, class _Container, class _Compare>
bool PriorityQueue<T, _Container, _Compare>::pop(T &data)
{
	std::unique_lock<std::mutex> lock(mMutex);

	if (!mQuit)
	{
		if (!mQueue.empty())
		{
            data = std::move(const_cast<T&>(mQueue.top()));
			mQueue.pop();

			mFull.notify_all();
			return true;
		}
		else if (mQueue.empty() && mFinished)
		{
			return false;
		}
		else
		{
			mEmpty.wait(lock);
		}
	}

	return false;
}

template <class T, class _Container, class _Compare>
bool PriorityQueue<T, _Container, _Compare>::popWait(T &data)
{
	std::unique_lock<std::mutex> lock(mMutex);

	while (!mQuit)
	{
		if (!mQueue.empty())
		{
			data = std::move(const_cast<T&>(mQueue.top()));
			mQueue.pop();

			mFull.notify_all();
			return true;
		}
		else if (mQueue.empty() && mFinished)
		{
			return false;
		}
		else
		{
			mEmpty.wait(lock);
		}
	}

	return false;
}

template <class T, class _Container, class _Compare>
bool PriorityQueue<T, _Container, _Compare>::popNoWait(T &data, bool &gotFrame)
{
	std::unique_lock<std::mutex> lock(mMutex);

	if (!mQuit)
	{
		if (!mQueue.empty())
		{
            data = std::move(const_cast<T&>(mQueue.top()));
			mQueue.pop();

			mFull.notify_all();
			gotFrame = true;
			return true;
		}
		else if (mQueue.empty() && mFinished)
		{
		    gotFrame = false;
			return false;
		}
		else
		{
		    gotFrame = false;
			return false;
		}
	}

	return false;
}

template <class T, class _Container, class _Compare>
void PriorityQueue<T, _Container, _Compare>::finished()
{
	mFinished.store(true);
	mEmpty.notify_all();
}

template <class T, class _Container, class _Compare>
void PriorityQueue<T, _Container, _Compare>::quit()
{
	mQuit.store(true);
	mEmpty.notify_all();
	mFull.notify_all();
}

template <class T, class _Container, class _Compare>
void PriorityQueue<T, _Container, _Compare>::flush()
{
    mQuit.store(true);
    mEmpty.notify_all();
    mFull.notify_all();

    //std::priority_queue<T, vector<T>, AVCompare<T>> empty;
    std::priority_queue<T, _Container, _Compare> empty;
    std::swap(mQueue, empty);

    reset();
}

template <class T, class _Container, class _Compare>
void PriorityQueue<T, _Container, _Compare>::reset()
{
    mFinished.store(false);
    mQuit.store(false);
	std::priority_queue<T, _Container, _Compare> empty;
	std::swap(mQueue, empty);
    mEmpty.notify_all();
    mFull.notify_all();
}

template <class T, class _Container, class _Compare>
void PriorityQueue<T, _Container, _Compare>::lock()
{
    mQuit.store(true);
    mEmpty.notify_all();
    mFull.notify_all();

    mMutex.lock();
}

template <class T, class _Container, class _Compare>
void PriorityQueue<T, _Container, _Compare>::unlock()
{
    mMutex.unlock();
    mEmpty.notify_all();
    mFull.notify_all();
}
