// ------------------------------------------------
// File : version2.h
// Date: 17-june-2005
// Author: giles
//
// (c) 2005 peercast.org
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

#ifndef _VERSION2_H
#define _VERSION2_H

// ------------------------------------------------
#ifdef PRIVATE_BROADCASTER
static const char PCP_BROADCAST_FLAGS	= 0x01;	
static bool	PCP_FORCE_YP				= true;
#else
static const char PCP_BROADCAST_FLAGS	= 0x00;
static bool	PCP_FORCE_YP				= false;
#endif
// ------------------------------------------------
static const int PCP_CLIENT_VERSION		= 1218;
static const int PCP_CLIENT_VERSION_VP	= 27;
static const int PCP_ROOT_VERSION		= 1218;

static const int PCP_CLIENT_MINVERSION	= 1200;

static const char *PCX_AGENT 		= "PeerCast/0.1218";	
static const char *PCX_AGENTJP 		= "PeerCast/0.1218-J";
static const char *PCX_AGENTVP		= "PeerCast/0.1218(VP0027)";
static const char *PCX_VERSTRING	= "v0.1218(VP0027)";

extern int version_ex; // VERSION_EX—LŒø‰»ƒtƒ‰ƒO

#if 1 /* for VP extend version */
//#define VERSION_EX 1
static const char *PCP_CLIENT_VERSION_EX_PREFIX = "IM"; // 2bytes only
static const int  PCP_CLIENT_VERSION_EX_NUMBER = 45;
static const char *PCX_AGENTEX = "PeerCast/0.1218(IM0045)";
static const char *PCX_VERSTRING_EX = "v0.1218(IM0045)";

static const char *PCP_CLIENT_DIST_URL = "http://pecaim.net/";
static const char *PCP_CLIENT_VERSION_URL = "version.pecaim.net";
#endif

// ------------------------------------------------


#endif
