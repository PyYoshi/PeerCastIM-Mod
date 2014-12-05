// Author: PyYoshi
// http://www.ibm.com/developerworks/linux/library/l-ipc2lin3/index.html
// http://blog.kmckk.com/archives/3962798.html

#ifndef _WIN32_COMPAT_H
#define _WIN32_COMPAT_H

#include <pthread.h>

typedef pthread_mutex_t CRITICAL_SECTION;

void InitializeCriticalSection(CRITICAL_SECTION *section);

void EnterCriticalSection(CRITICAL_SECTION *section);

void LeaveCriticalSection(CRITICAL_SECTION *section);

void DeleteCriticalSection(CRITICAL_SECTION *section);

#ifdef OS_MACOSX
#define PTHREAD_MUTEX_RECURSIVE_VALUE PTHREAD_MUTEX_RECURSIVE
#endif

#ifdef OS_LINUX
#define PTHREAD_MUTEX_RECURSIVE_VALUE PTHREAD_MUTEX_RECURSIVE_NP
#endif

#endif
