#ifndef KOMUTEX_H_2015_09_18
#define KOMUTEX_H_2015_09_18

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#include <winsock2.h>
#include <windows.h>

class commonMutex
{
public:
	commonMutex()
	{
		InitializeCriticalSection(&cs);
	}
	~commonMutex()
	{
		DeleteCriticalSection(&cs);
	}

	void lock()
	{
		EnterCriticalSection(&cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
	}

private:
	CRITICAL_SECTION cs;
};

#else // defined(__linux__) || defined(__linux)
#include <pthread.h>

class commonMutex
{
public:
	commonMutex()
	{
		pthread_mutex_init(&mutex, NULL);
	}
	~commonMutex()
	{
		pthread_mutex_destroy(&mutex);
	}
	void lock()
	{
		pthread_mutex_lock(&mutex);
	}

	void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_t* getmutex()
	{
		return &mutex;
	}

public:
	pthread_mutex_t mutex;
};
#endif

class AutoLocker {
public:
	AutoLocker(commonMutex* locker)
	{
		m_pLocker = locker;
		if (m_pLocker)
			m_pLocker->lock();
	};

	~AutoLocker() {
		if (m_pLocker)
		{
			m_pLocker->unlock();
			m_pLocker = NULL;
		}
	}

private:
	commonMutex* m_pLocker;
};


#endif