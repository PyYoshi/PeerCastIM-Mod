/*
 * Simple vector (Thread safe implementation)
 *                        for Linux
 *
 *                               Impl. by PyYoshi
 */

#ifndef _CORELIB_LINUX_TS_VECTOR_H_
#define _CORELIB_LINUX_TS_VECTOR_H_

#include <stdlib.h>
#include <stdexcept>
#include "unix/compat.h"
#include "common/ts_vector.h"

template<class T>
class LTSVector : public ITSVector<T> {
private:
    CRITICAL_SECTION csec;

public:
    LTSVector() {
        InitializeCriticalSection(&csec);
    }

    ~LTSVector() {
        DeleteCriticalSection(&csec);
    }

    inline virtual void lock() {
        EnterCriticalSection(&csec);
    }

    inline virtual void unlock() {
        LeaveCriticalSection(&csec);
    }
};

#endif
