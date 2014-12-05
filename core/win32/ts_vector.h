/*
 * Simple vector (Thread safe implementation)
 *                        for Windows w/ VC++
 *
 *                               Impl. by Eru
 */

#ifndef _CORELIB_WIN32_TS_VECTOR_H_
#define _CORELIB_WIN32_TS_VECTOR_H_

#include <stdlib.h>
#include <stdexcept>
#include <windows.h>
#include "common/ts_vector.h"

template <class T>
class WTSVector : public ITSVector<T>
{
private:
	CRITICAL_SECTION csec;

public:
	WTSVector()
	{
		InitializeCriticalSection(&csec);
	}

	~WTSVector()
	{
		DeleteCriticalSection(&csec);
	}

	inline virtual void lock()
	{
		EnterCriticalSection(&csec);
	}

	inline virtual void unlock()
	{
		LeaveCriticalSection(&csec);
	}
};

#endif
