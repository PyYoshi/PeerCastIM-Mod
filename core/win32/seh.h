#ifndef _SEH_H_
#define _SEH_H_

#include "stream.h"
#include <dbghelp.h>

#pragma once
#pragma comment(lib, "dbghelp.lib")

extern FileStream fs;

#ifndef _DEBUG
#define SEH_THREAD(func, name) \
{ \
	__try \
	{ \
		return func(thread); \
	} __except(SEHdump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER) \
	{ \
	} \
}

#else

#define SEH_THREAD(func, name) return func(thread);
#endif

void SEHdump(_EXCEPTION_POINTERS *);

#endif
