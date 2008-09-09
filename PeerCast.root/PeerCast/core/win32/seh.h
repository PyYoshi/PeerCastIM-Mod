#ifndef _SEH_H_
#define _SEH_H_

#include "stream.h"
#include <dbghelp.h>

#pragma once
#pragma comment(lib, "dbghelp.lib")

extern FileStream fs;

#define SEH_THREAD(func, name) \
{ \
	__try \
	{ \
		return func(thread); \
	} __except(SEHdump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER) \
	{ \
	} \
} \

void SEHdump(_EXCEPTION_POINTERS *);

#endif
