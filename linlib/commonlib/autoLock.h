#ifndef AUTO_LOCKER_H
#define AUTO_LOCKER_H

#include "commonMutex.h"


class autoLock {
public:
	autoLock(commonMutex &m):m_mutex(m)
	{
		m_mutex.lock();
	};

	~autoLock()
	{
		m_mutex.unlock();
	};

private:
	commonMutex &m_mutex;
};


#endif