// ------------------------------------------------
// File : rtsp.h
// Date: 4-apr-2002
// Author: giles
// Desc: 
//
// (c) 2002 peercast.org
// ------------------------------------------------
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ------------------------------------------------

#ifndef _RTSP_H
#define _RTSP_H

#include "http.h"

// ---------------------------------------------
class RTSP : public HTTP
{
public:
	RTSP(Stream &s):HTTP(s){}
};

#endif
