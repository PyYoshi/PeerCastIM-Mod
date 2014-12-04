// Author: PyYoshi
// http://www.ibm.com/developerworks/linux/library/l-ipc2lin3/index.html
// http://blog.kmckk.com/archives/3962798.html

#include "unix/compat.h"

void InitializeCriticalSection(CRITICAL_SECTION *section)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr); /* 2012/07/26 追記 */
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_VALUE);
    pthread_mutex_init(section, &attr);
    pthread_mutexattr_destroy(&attr);
}
void EnterCriticalSection(CRITICAL_SECTION *section)
{
    pthread_mutex_lock(section);
}
void LeaveCriticalSection(CRITICAL_SECTION *section)
{
    pthread_mutex_unlock(section);
}
void DeleteCriticalSection(CRITICAL_SECTION *section)
{
    pthread_mutex_destroy(section);
}
