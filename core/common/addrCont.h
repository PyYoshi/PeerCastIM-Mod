/*
 * Container type for IP-addr blacklist
 * 
 *                        Impl. by Eru
 */

#ifndef _CORELIB_COMMON_ADDRCONT_H_
#define _CORELIB_COMMON_ADDRCONT_H_

#include <limits.h>

class addrCont
{
public:
	unsigned int addr;
	unsigned int count;

	addrCont() : addr(0), count(0)
	{
	}

	addrCont(unsigned int addr) : addr(addr), count(0)
	{
	}

	addrCont& operator++()
	{
		if (this->count < UINT_MAX)
			++(this->count);

		return *this;
	}

	inline bool operator==(const addrCont &op)
	{
		return this->addr == op.addr;
	}

	inline bool operator==(const unsigned int op)
	{
		return this->addr == op;
	}
};

#endif
