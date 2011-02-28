// ------------------------------------------------
// File : gui.h
// Date: 4-apr-2002
// Author: giles
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

#ifndef _GUI_H
#define _GUI_H

#include "sys.h"
#include "gdiplus.h"
#include "channel.h"
#include "servent.h"

extern LRESULT CALLBACK GUIProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern void ADDLOG(const char *str,int id,bool sel,void *data, LogBuffer::TYPE type);

extern String iniFileName;
extern HWND guiWnd;
extern int logID;

enum 
{
	WM_INITSETTINGS = WM_USER,
	WM_GETPORTNUMBER,
	WM_PLAYCHANNEL,
	WM_TRAYICON,
	WM_SHOWGUI,
	WM_SHOWMENU,
	WM_PROCURL,
	WM_UPDATETRAFFIC
};

class IdData
{
private:
	int channel_id;
	int servent_id;
	unsigned int ip_addr;

public:
	IdData(int cid, int sid, unsigned int ip){
		channel_id = cid;
		servent_id = sid;
		ip_addr = ip;
	}

	int getChannelId(){return channel_id;};
	int getServentId(){return servent_id;};
	unsigned int getIpAddr(){return ip_addr;};
};

class ServentData
{
private:
	int servent_id;
	int type;
//	unsigned int tnum;
	int status;
//	String agent;
	Host host;
	String hostname;
//	unsigned int syncpos;
//	char *typeStr;
//	char *statusStr;
//	bool infoFlg;
//	bool relay;
//	bool firewalled;
	unsigned int totalRelays;
	unsigned int totalListeners;
//	int vp_ver;
//	char ver_ex_prefix[2];
//	int ver_ex_number;

	bool EnableFlg;
	bool infoFlg;
	ServentData *next;
	ChanHit chanHit;
	bool selected;
	int posX;
	int posY;
	int	width;
	int height;

	unsigned int lastSkipTime;
	unsigned int lastSkipCount;

public:
	ServentData(){
		next = NULL;
		EnableFlg = false;
		infoFlg = false;

		posX = 0;
		posY = 0;
		width = 0;
		selected = false;

	}
	void setData(Servent *s, ChanHit *hit, unsigned int listeners, unsigned int relays, bool infoFlg);
	bool getInfoFlg(){return infoFlg;}
	ChanHit *getChanHit(){return &chanHit;};
	int getStatus(){return status;};
	Host getHost(){return host;};

	int getServentId(){return servent_id;};

	bool getEnableFlg(){return EnableFlg;};
	void setEnableFlg(bool f){EnableFlg = f;};
	ServentData *getNextData(){return next;};
	void setNextData(ServentData *s){next = s;};
	bool getSelected(){return selected;};
	void setSelected(bool f){selected = f; if (!f){posX=0;posY=0;}};
	int getWidth(){return width;};
	void setWidth(int w){width = w;};
	int getHeight(){return height;};
	void setHeight(int h){height = h;};
	String getName(){return hostname;};
	void setName(String n){hostname = n;};

	int drawServent(Gdiplus::Graphics *g, int x, int y);

	bool checkDown(int x, int y);

};

class ChannelData {

private:
	int channel_id;
	char name[257];
	int bitRate;
	unsigned int lastPlayStart;
	int status;
	int totalListeners;
	int totalRelays;
	int localListeners;
	int localRelays;
	bool stayConnected;
	ChanHit chDisp;
	bool bTracker;
	unsigned int lastSkipTime;
	unsigned int skipCount;

	bool EnableFlg;
	ChannelData *next;

	int posX;
	int posY;
	int width;
	int height;
	bool selected;
	ServentData *serventDataTop;
	bool openFlg;

public:
	ChannelData(){
		EnableFlg = FALSE;
		next = NULL;
		posX = 0;
		posY = 0;
		width = 0;
		height = 0;
		selected = FALSE;
		serventDataTop = NULL;
		openFlg = FALSE;
		type = Servent::T_NONE;
		servent_id = -1;
	}
	int drawChannel(Gdiplus::Graphics *g, int x, int y);

	void setData(Channel *);
	int getChannelId(){return channel_id;};
	char* getName(){return &(name[0]);};
	int getBitRate(){return bitRate;};
	bool isStayConnected(){return stayConnected;};
	bool isTracker(){return bTracker;};
	int getStatus(){return status;};
	unsigned int getLastSkipTime(){return lastSkipTime;};

	int getTotalListeners(){return totalListeners;};
	int getTotalRelays(){return totalRelays;};
	int getLocalListeners(){return localListeners;};
	int getLocalRelays(){return localRelays;};

	bool getEnableFlg(){return EnableFlg;};
	void setEnableFlg(bool flg){EnableFlg = flg;};
	ChannelData *getNextData(){return next;};
	void setNextData(ChannelData* cd){next = cd;};

	int getPosX(){return posX;};
	void setPosX(int x){posX = x;}
	int getPosY(){return posY;};
	void setPosY(int y){posY = y;};
	int getWidth(){return width;};
	void setWidth(int w){width = w;};
	int getHeight(){return height;};
	void setHeight(int h){height = h;};
	bool isSelected(){return selected;};
	void setSelected(bool sel){selected = sel;};
	bool getOpenFlg(){return openFlg;};
	void setOpenFlg(bool b){openFlg = b;};

	ServentData* getServentDataTop(){return serventDataTop;};
	ServentData* findServentData(int servent_id);
	void addServentData(ServentData *sd);
	void deleteDisableServents();

	bool setName(int servent_id, String name);
	int getServentCount();

	bool checkDown(int x, int y);

	Servent::TYPE type; // COUTのサーバント情報保持用
	int servent_id; // 同上。channel_idで代用できたけどPublicにしたくない
};



#endif