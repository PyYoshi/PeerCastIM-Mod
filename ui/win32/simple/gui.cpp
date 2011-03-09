// ------------------------------------------------
// File : gui.cpp
// Date: 4-apr-2002
// Author: giles
// Desc: 
//		Windows front end GUI, PeerCast core is not dependant on any of this. 
//		Its very messy at the moment, but then again Windows UI always is.
//		I really don`t like programming win32 UI.. I want my borland back..
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

#define _WIN32_WINNT 0x0500

#include "ws2tcpip.h" // getnameinfo
#include "wspiapi.h" // compatibility for Win2k and earlier

#include <windows.h>
#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "resource.h"
#include "socket.h"
#include "win32/wsys.h"
#include "servent.h"
#include "win32/wsocket.h"
#include "inifile.h"
#include "gui.h"
#include "servmgr.h"
#include "peercast.h"
#include "simple.h"
#include "gdiplus.h"
#include "commctrl.h"
#include "locale.h"
#include "stats.h"
#include "socket.h"
#include "wininet.h"
#ifdef _DEBUG
#include "chkMemoryLeak.h"
#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

ThreadInfo guiThread;
bool shownChannels=false;
WINDOWPLACEMENT winPlace;
bool guiFlg = false;

extern bool jumpListEnabled;

using namespace Gdiplus;

#include <comdef.h>

void APICALL MyPeercastApp ::printLog(LogBuffer::TYPE t, const char *str)
{
/*	ADDLOG(str,logID,true,NULL,t);
	if (logFile.isOpen())
	{
		logFile.writeLine(str);
		logFile.flush();
	}*/
}

void APICALL MyPeercastApp::updateSettings()
{
//	setControls(true);
}

Gdiplus::Bitmap bmpBack(800,600,PixelFormat24bppRGB);
UINT backWidth;
UINT backHeight;

Gdiplus::Image *backImage;
Gdiplus::Bitmap *backBmp;
Gdiplus::Graphics *backGra;

Gdiplus::Image *img_idle;
Gdiplus::Image *img_connect;
Gdiplus::Image *img_conn_ok;
Gdiplus::Image *img_conn_full;
Gdiplus::Image *img_conn_over;
Gdiplus::Image *img_conn_ok_skip;
Gdiplus::Image *img_conn_full_skip;
Gdiplus::Image *img_conn_over_skip;
Gdiplus::Image *img_error;
Gdiplus::Image *img_broad_ok;
Gdiplus::Image *img_broad_full;

UINT winWidth=0;
UINT winHeight=0;

static HWND hTree;
extern HINSTANCE hInst;
extern HWND guiWnd;
extern Stats stats;

WLock sd_lock;
WLock ChannelDataLock;
WLock MakeBackLock;
ChannelData *channelDataTop = NULL;

bool gbDispTop = false;
bool gbAllOpen = false;

THREAD_PROC GetHostName(ThreadInfo *thread){
	IdData *id = (IdData*)(thread->data);

	//HOSTENT *he;
	u_long ip;
	struct sockaddr_in sa;
	char host[256];
	char *tmp;
	bool flg = TRUE;
	bool findFlg;
	int error;

	ip = htonl(id->getIpAddr());

	memset(&sa, 0, sizeof(sa));
	sa.sin_addr.S_un.S_addr = ip;
	sa.sin_family = AF_INET;

	for (int i=0; i<10 && flg; i++){
		error = getnameinfo(reinterpret_cast<sockaddr*>(&sa), sizeof(sa), host, sizeof(host)/sizeof(host[0]), NULL, 0, NI_NAMEREQD);
		switch (error)
		{
		case 0:
			// success
			flg = FALSE;
			break;

		case WSAHOST_NOT_FOUND:
			LOG_ERROR("cannot resolve host for %s",
				((tmp = inet_ntoa(sa.sin_addr)) ? tmp : ""));
			flg = TRUE;
			break;

		default:
			LOG_ERROR("an error occurred while resolving hostname of %s (%ld)",
				((tmp = inet_ntoa(sa.sin_addr)) ? tmp : ""), error);
		}
	}

	if (error)
		return 0;

	for (flg=TRUE, findFlg=FALSE; flg; )
	{
		ChannelDataLock.on();
		ChannelData* cd = channelDataTop;

		while(cd){
			if (cd->getChannelId() == id->getChannelId())
			{
				findFlg = TRUE;

				if (cd->findServentData(id->getServentId()))
				{
					if (cd->setName(id->getServentId(), host))
					{
						LOG_DEBUG("successfully resolved(%d)", id->getServentId());
						flg = FALSE;
						break;
					} else
					{
						LOG_ERROR("cannot update servent data with resolved information");
						flg = FALSE;
						break;
					}
				} else
				{
					LOG_DEBUG("servent data has been removed");
					flg = FALSE;
					break;
				}
			}

			cd = cd->getNextData();
		}

//		::delete id;
		ChannelDataLock.off();

		if (!findFlg)
		{
			LOG_DEBUG("servent data has been removed(channel)");
			flg = FALSE;
		}

		sys->sleep(1000);
	}


	return 0;
}

int drawSpeed(Graphics *gra, int posX, int posY){

	// ���x�\�����̔w�i�𔒂�����
	SolidBrush b(Color(180,255,255,255));
	backGra->FillRectangle(&b, posX, posY, 200, 14);
	// �t�H���g�ݒ�
	Font font(L"�l�r �o�S�V�b�N",10);
	// �����F
	SolidBrush strBrush(Color::Black);
	// ������쐬
	char tmp[256];
	sprintf(tmp, "R:%.1fkbps S:%.1fkbps", 
		BYTES_TO_KBPS(stats.getPerSecond(Stats::BYTESIN)-stats.getPerSecond(Stats::LOCALBYTESIN)),
		BYTES_TO_KBPS(stats.getPerSecond(Stats::BYTESOUT)-stats.getPerSecond(Stats::LOCALBYTESOUT)));
	_bstr_t bstr(tmp);
	// �����\���͈͎w��
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	RectF r((REAL)posX, (REAL)posY, (REAL)200, (REAL)14);
	// �����`��
	gra->DrawString(bstr, -1, &font, r, &format, &strBrush);



	return posY + 15;
}

void MakeBack(HWND hwnd, UINT x, UINT y){
	MakeBackLock.on();

	winWidth = x;
	winHeight = y;

	if (backGra){
		::delete backBmp;
		::delete backGra;
	}

	backBmp = ::new Bitmap(x,y);
	backGra = ::new Graphics(backBmp);

	// �S�Ĕ��œh��Ԃ�
	SolidBrush b(Color(255,255,255,255));
	backGra->FillRectangle(&b, 0, 0, x, y);

	backWidth = backImage->GetWidth();
	backHeight = backImage->GetHeight();

	// �w�i�摜��`��
	for (UINT xx = 0; xx < x/backWidth + 1; xx++){
		for (UINT yy = 0; yy < y/backHeight + 1; yy++){
			UINT width,height;
			if (backWidth*(xx+1) > x){
				width = x % backWidth;
			} else {
				width = backWidth;
			}
			if (backHeight*(yy+1) > y){
				height = y % backHeight;
			} else {
				height = backHeight;
			}
			Rect r((INT)backWidth*xx, (INT)backHeight*yy, width, height);
			backGra->DrawImage(backImage, r, 0, 0, (INT)width, (INT)height, UnitPixel);
		}
	}

	INT posX = 20;
	INT posY = 20;

	// ���x�`��
	drawSpeed(backGra, winWidth-205, 5);

	// �`�����l������`��
	ChannelDataLock.on();
	ChannelData *cd = channelDataTop;
	while(cd){
		posY = cd->drawChannel(backGra, 20, posY);
		cd = cd->getNextData();
	}
	ChannelDataLock.off();
	MakeBackLock.off();
}

void MakeBack(HWND hwnd){
	MakeBack(hwnd, winWidth, winHeight);
	::InvalidateRect(guiWnd, NULL, FALSE);
}

void ChannelData::setData(Channel *c){
	String sjis;
	sjis = c->getName();
	sjis.convertTo(String::T_SJIS);

	strncpy(name, sjis, 256);
	name[256] = '\0';
	channel_id = c->channel_id;
	bitRate = c->info.bitrate;
	lastPlayStart = c->info.lastPlayStart;
	status = c->status;
	totalListeners = c->totalListeners();
	totalRelays = c->totalRelays();
	localListeners = c->localListeners();
	localRelays = c->localRelays();
	stayConnected = c->stayConnected;
	chDisp = c->chDisp;
	bTracker = c->sourceHost.tracker;
	lastSkipTime = c->lastSkipTime;
	skipCount = c->skipCount;
}

int gW = 0;
int gWS = 0;

int ChannelData::drawChannel(Graphics *g, int x, int y){
	REAL xx = x * 1.0f;
	REAL yy = y * 1.0f;
	ServentData* sd;

	// �ʒu��ۑ�
	posX = x;
	posY = y;

	int w/*,h*/;

	if (getWidth() == 0){
		if (gW){
			w = gW;
		} else {
			w = 400;
		}
	} else {
		w = getWidth();
		gW = w;
	}

	// �`�����l���\�����̔w�i��h��
	if (isSelected()){
		// �I��
		SolidBrush b(Color(160,49,106,197));
		g->FillRectangle(&b, x, y, w, 14);
	} else {
		// ��I��
		SolidBrush b(Color(160,255,255,255));
		g->FillRectangle(&b, x, y, w, 14);
	}

	// �X�e�[�^�X�\��
	Gdiplus::Image *img = NULL;
	unsigned int nowTime = sys->getTime();
	if (this->type != Servent::T_COUT)
	{
		// COUT�ȊO
		Channel *ch = chanMgr->findChannelByChannelID(this->channel_id);
		switch(this->getStatus()){
		case Channel::S_IDLE:
			img = img_idle;
			break;
		case Channel::S_SEARCHING:
		case Channel::S_CONNECTING:
			img = img_connect;
			break;
		case Channel::S_RECEIVING:
			if ((skipCount > 2) && (lastSkipTime + 120 > nowTime)){
				if (chDisp.relay){
					img = img_conn_ok_skip;
				} else {
					if (chDisp.numRelays){
						img = img_conn_full_skip;
					} else {
						img = img_conn_over_skip;
					}
				}
			} else {
				if (chDisp.relay){
					img = img_conn_ok;
				} else {
					if (chDisp.numRelays){
						img = img_conn_full;
					} else {
						img = img_conn_over;
					}
				}
			}
			break;
		case Channel::S_BROADCASTING:
			img = img_broad_ok;
			break;
		case Channel::S_ERROR:
			// bump���ɃG���[���\�������̂�h�~
			if (ch && ch->bumped)
			{
				img = img_connect;
			} else
			{
				img = img_error;
			}
			break;
		default:
			img = img_idle;
			break;
		}
	} else
	{
		// COUT�p
		img = img_broad_ok;
	}

	// �`���_
	PointF origin(xx, yy);
	// �X�e�[�^�X�\���ʒu
	Rect img_rect((INT)origin.X, (INT)origin.Y + 1, img ? img->GetWidth() : 12, 12);
	// �X�e�[�^�X�`��
	ImageAttributes att;
//	att.SetColorKey(Color::White, Color::White, ColorAdjustTypeBitmap);
	g->DrawImage(img, img_rect, 0, 0, img_rect.Width, 12, UnitPixel, &att);
	// ���̊�_
	origin.X += img_rect.Width;

	// �t�H���g�ݒ�
	Gdiplus::Font font(L"�l�r �o�S�V�b�N",10);
	// �����F
	SolidBrush *strBrush;
	if (servMgr->getFirewall() == ServMgr::FW_ON){
		strBrush = ::new SolidBrush(Color::Red);
	} else if (isTracker() && (getStatus() == Channel::S_RECEIVING)){
		strBrush = ::new SolidBrush(Color::Green);
	} else {
		if (isSelected()){
			// �I��
			strBrush = ::new SolidBrush(Color::White);
		} else {
			// ��I��
			strBrush = ::new SolidBrush(Color::Black);
		}
	}

	if (this->type != Servent::T_COUT)
	{
		// COUT�ȊO

		// �`�����l�����\��
		g->SetTextRenderingHint(TextRenderingHintAntiAlias);
		_bstr_t bstr1(getName());
		// �����`��͈͎w��
		RectF r1(origin.X, origin.Y, 120.0f, 13.0f);
		StringFormat format;
		format.SetAlignment(StringAlignmentNear);
		g->DrawString(bstr1, -1, &font, r1, &format, strBrush);
		// ���̊�_
		origin.X += r1.Width;

		//// �㗬IP/���X�i�[��/�����[���\��
		//// NOTE:
		////    �҂������̓���׋��p�B�����[�X�r���h�ł͌��̃R�[�h���g�p�̎��B
		////    �����\���͈͕͂�220���炢�ł���
		//char tmp[512]; // �\���p�o�b�t�@
		//char hostip[256]; // IP�A�h���X�o�b�t�@
		//chDisp.uphost.toStr(hostip); // �㗬IP
		//sprintf(tmp, "%d/%d - [%d/%d] - %s",
		//	getTotalListeners(),
		//	getTotalRelays(),
		//	getLocalListeners(),
		//	getLocalRelays(),
		//	hostip
		//	);

		// ���X�i�[��/�����[���\��
		char tmp[256];
		sprintf(tmp, "%d/%d - [%d/%d]", getTotalListeners(), getTotalRelays(), getLocalListeners(), getLocalRelays());
		_bstr_t bstr2(tmp);
		// �����\���͈͎w��
		RectF r2(origin.X, origin.Y, 100.0f, 13.0f);
		format.SetAlignment(StringAlignmentCenter);
		g->DrawString(bstr2, -1, &font, r2, &format, strBrush);
		// ���̊�_
		origin.X += r2.Width;

		// bps�\��
		Font *f;
		if (isStayConnected()){
			f = ::new Font(L"Arial", 9.0f, FontStyleItalic|FontStyleBold, UnitPoint);
		} else {
			f = ::new Font(L"Arial", 9.0f);
		}
		sprintf(tmp, "%dkbps", getBitRate());
		_bstr_t bstr3(tmp);
		format.SetAlignment(StringAlignmentFar);
		// �����\���͈͎w��
		RectF r3(origin.X, origin.Y, 80.0f, 13.0f);
		g->DrawString(bstr3, -1, f, r3, &format, strBrush);
		// �t�H���g�J��
		::delete f;

		// ���̊�_
		origin.X += r3.Width;

		// �u���V�폜
		::delete strBrush;


		// Servent�\��
		if (!openFlg){
			int count = getServentCount();
			// Servent�\�����̔w�i�𔒂ɂ���
			SolidBrush b(Color(160,255,255,255));
			g->FillRectangle(&b, (INT)origin.X, (INT)origin.Y, 14*count, 14);

			sd = serventDataTop;
			int index = 0;
			while(sd){
				SolidBrush *serventBrush;
				if (sd->getInfoFlg()){
					ChanHit *hit = sd->getChanHit();
					if (hit->firewalled){
						SolidBrush bb(Color(180,255,0,0));
						g->FillRectangle(&bb, (INT)origin.X + 14*index, (INT)origin.Y, 14, 14);
					}
					if (hit->relay){
						// �����[�n�j
						serventBrush = ::new SolidBrush(Color::Green);
					} else {
						// �����[�s��
						if (hit->numRelays){
							// �����[��t
							serventBrush = ::new SolidBrush(Color::Blue);
						} else {
							// �����[�Ȃ�
							serventBrush = ::new SolidBrush(Color::Purple);
						}
					}
				} else {
					// ���Ȃ�
					serventBrush = ::new SolidBrush(Color::Black);
				}
				// �l�p�`��
				backGra->FillRectangle(serventBrush, (INT)origin.X + index*14 + 1, (INT)origin.Y + 1, 12, 12);

				::delete serventBrush;
				sd = sd->getNextData();
				index++;
			}
		}

		// ���̊�_
		origin.Y += 15;

		// �T�C�Y��ۑ�
		setWidth((int)origin.X - posX);
		setHeight((int)origin.Y - posY);

		// ServentData�\��
		sd = serventDataTop;
		while(sd){
			if (openFlg || sd->getSelected()){
				sd->drawServent(g, (INT)x+12, (INT)origin.Y);
				// ���̊�_
				origin.Y += 15;
			}
			sd = sd->getNextData();
		}
	} else
	{
		// COUT
		g->SetTextRenderingHint(TextRenderingHintAntiAlias);
		RectF r1(origin.X, origin.Y, 120.0f+100.0f+80.0f, 13.0f);
		origin.X += r1.Width;
		StringFormat format;
		format.SetAlignment(StringAlignmentNear);
		_bstr_t bstr1("COUT");
		g->DrawString(bstr1, -1, &font, r1, &format, strBrush);
		::delete strBrush;
		origin.Y += 15;
		setWidth((int)origin.X - posX);
		setHeight((int)origin.Y - posY);
	}

	return (int)(origin.Y);
}

bool ChannelData::checkDown(int x,int y){
	// �͈͓��`�F�b�N
	if (	
			(x > posX)
		&&	(x < posX + getWidth())
		&&	(y > posY)
		&&	(y < posY + getHeight())
	){
		return TRUE;
	}
	return FALSE;
}

ServentData *ChannelData::findServentData(int servent_id){
	ServentData *sv = serventDataTop;
	while(sv){
		if (sv->getServentId() == servent_id){
			return sv;
		}
		sv = sv->getNextData();
	}
	return NULL;
}

void ChannelData::addServentData(ServentData *sd){
	sd->setNextData(serventDataTop);
	serventDataTop = sd;
}

void ChannelData::deleteDisableServents(){
	ServentData *sd = serventDataTop;
	ServentData *prev = NULL;

	while(sd){
		if (!(sd->getEnableFlg())){
			ServentData *next = sd->getNextData();
			if (prev){
				prev->setNextData(next);
			} else {
				serventDataTop = next;
			}
			::delete sd;
			sd = next;
		} else {
			prev = sd;
			sd = sd->getNextData();
		}
	}
}

int ChannelData::getServentCount(){
	int ret = 0;

	ServentData *sd = serventDataTop;
	while(sd){
		ret++;
		sd = sd->getNextData();
	}
	return ret;
}

bool ChannelData::setName(int servent_id, String name){
	ServentData *sd = serventDataTop;
	while(sd){
		if (sd->getServentId() == servent_id){
			sd->setName(name);
			return TRUE;
		}
		sd = sd->getNextData();
	}
	return FALSE;
}

void ServentData::setData(Servent *s, ChanHit *hit, unsigned int listeners, unsigned int relays, bool f){
	servent_id = s->servent_id;
	type = s->type;
	status = s->status;
	lastSkipTime = s->lastSkipTime;
	host = s->getHost();

	chanHit.numRelays = hit->numRelays;
	chanHit.relay = hit->relay;
	chanHit.firewalled = hit->firewalled;
	chanHit.version = hit->version;
	chanHit.version_vp = hit->version_vp;
	chanHit.version_ex_number = hit->version_ex_number;
	chanHit.version_ex_prefix[0] = hit->version_ex_prefix[0];
	chanHit.version_ex_prefix[1] = hit->version_ex_prefix[1];

	totalListeners = listeners;
	totalRelays = relays;

	infoFlg = f;
}

int ServentData::drawServent(Gdiplus::Graphics *g, int x, int y){
	REAL xx = x * 1.0f;
	REAL yy = y * 1.0f;
	int w/*,h*/;

	// �ʒu��ۑ�
	posX = x;
	posY = y;

	if (getWidth() == 0){
		if (gWS){
			w = gWS;
		} else {
			w = 400;
		}
	} else {
		w = getWidth();
		gWS = w;
	}

	// �`���_
	PointF origin(xx, yy);

	// �t�H���g�ݒ�
	Font font(L"�l�r �o�S�V�b�N",9);
	// �����F
	SolidBrush *strBrush;
	if (chanHit.firewalled){
		strBrush = ::new SolidBrush(Color::Red);
	} else {
		if (getSelected()){
			// �I��
			strBrush = ::new SolidBrush(Color::White);
		} else {
			// ��I��
			strBrush = ::new SolidBrush(Color::Black);
		}
	}
	// ServantData�\��
	g->SetTextRenderingHint(TextRenderingHintAntiAlias);
	// ������쐬
	char tmp[256];
	char host1[256];
	host.toStr(host1);

	if (infoFlg){
		if (chanHit.version_ex_number){
			// �g���o�[�W����
			sprintf(tmp, "%c%c%04d - %d/%d - %s(%s)", 
				chanHit.version_ex_prefix[0],
				chanHit.version_ex_prefix[1],
				chanHit.version_ex_number,
				totalListeners,
				totalRelays,
				host1,
				hostname.cstr()
				);
		} else if (chanHit.version_vp){
			sprintf(tmp, "VP%04d - %d/%d - %s(%s)", 
				chanHit.version_vp,
				totalListeners,
				totalRelays,
				host1,
				hostname.cstr()
				);
		} else {
			sprintf(tmp, "(-----) - %d/%d - %s(%s)",
				totalListeners,
				totalRelays,
				host1,
				hostname.cstr()
				);
		}
	} else {
			sprintf(tmp, "(-----) - %d/%d - %s(%s)",
				totalListeners,
				totalRelays,
				host1,
				hostname.cstr()
				);
	}
	_bstr_t bstr1(tmp);

	// �X�e�[�^�X�\��
	Gdiplus::Image *img = NULL;
	unsigned int nowTime = sys->getTime();
	switch(getStatus()){
		case Servent::S_CONNECTING:
			img = img_connect;
			break;
		case Servent::S_CONNECTED:
			if (lastSkipTime + 120 > nowTime){
				if (chanHit.relay){
					img = img_conn_ok_skip;
				} else {
					if (chanHit.numRelays){
						img = img_conn_full_skip;
					} else {
						img = img_conn_over_skip;
					}
				}
			} else {
				if (chanHit.relay){
					img = img_conn_ok;
				} else {
					if (chanHit.numRelays){
						img = img_conn_full;
					} else {
						img = img_conn_over;
					}
				}
			}
			break;
		default:
			break;
	}

	// �����`��͈͎w��
	RectF r1(origin.X + img->GetWidth() + 2, origin.Y, 800.0f, 13.0f);
	RectF r2;
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	g->MeasureString(bstr1, -1, &font, r1, &format, &r2);

	w = (INT)r2.Width + img->GetWidth() + 2;
	// ServentData�\�����̔w�i��h��
	if (getSelected()){
		// �I��
		SolidBrush b(Color(160,49,106,197));
		g->FillRectangle(&b, x, y, w, 13);
	} else {
		// ��I��
		SolidBrush b(Color(160,200,200,200));
		g->FillRectangle(&b, x, y, w, 13);
	}

	// �X�e�[�^�X�\���ʒu
	Rect img_rect((INT)origin.X, (INT)origin.Y+1, img ? img->GetWidth() : 12, 12);
	// �X�e�[�^�X�`��
	ImageAttributes att;
//	att.SetColorKey(Color::White, Color::White, ColorAdjustTypeBitmap);
	g->DrawImage(img, img_rect, 0, 0, img_rect.Width, 12, UnitPixel, &att);
	// ���̊�_
	origin.X += 12;

	g->DrawString(bstr1, -1, &font, r2, &format, strBrush);
	// ���̊�_
	origin.X += r2.Width;
	origin.Y += 13;

	setWidth((int)origin.X-posX);
	setHeight((int)origin.Y - posY);

	::delete strBrush;
	return 0;
}

bool ServentData::checkDown(int x, int y){
	if (	
			(x > posX)
		&&	(x < posX + getWidth())
		&&	(y > posY)
		&&	(y < posY + getHeight())
	){
		return TRUE;
	}
	return FALSE;
}


THREAD_PROC GUIDataUpdate(ThreadInfo *thread){
	int i;

	// set GUI thread status to running
	thread->finish = false;

	while(thread->active){
		// �`�����l���f�[�^���b�N
		ChannelDataLock.on();
		// �`�����l���f�[�^�̍X�V�t���O��S��FALSE�ɂ���
		ChannelData *cd = channelDataTop;
		while(cd){
			// Servent�̍X�V�t���O��FALSE�ɂ���
			ServentData *sv = cd->getServentDataTop();
			while(sv){
				sv->setEnableFlg(FALSE);
				sv = sv->getNextData();
			}
			cd->setEnableFlg(FALSE);
			cd = cd->getNextData();
		}

		Channel *c = chanMgr->channel;
		// ���ݑ��݂���`�����l�������[�v
		while(c){
			// ���Ƀ`�����l���f�[�^�������Ă��邩
			cd = channelDataTop;
			// �����t���OFALSE
			bool bFoundFlg = FALSE;
			while(cd){
				if (cd->getChannelId() == c->channel_id){
					//���Ƀ`�����l���f�[�^������̂ŁA���̂܂܍X�V
					cd->setData(c);
					// �X�V�t���OTRUE
					cd->setEnableFlg(TRUE);
					// �����t���OTRUE
					bFoundFlg = TRUE;
					// ���[�v���E
					break;
				}
				// ������Ȃ������ꍇ�A���̃f�[�^���`�F�b�N
				cd = cd->getNextData();
			}

			// �V�����`�����l���̏ꍇ�A�V�K�f�[�^�쐬
			if (!bFoundFlg){
				// �V�K�f�[�^�쐬
				cd = ::new ChannelData();
				// �f�[�^�X�V
				cd->setData(c);
				// �X�V�t���OTRUE
				cd->setEnableFlg(TRUE);

				// �V�K�f�[�^�����X�g�̐擪�ɓ����
				cd->setNextData(channelDataTop);
				channelDataTop = cd;
			}
			// ���̃`�����l�����擾
			c = c->next;
		}

#if 1
		// COUT������
		{
			bool foundFlg = false;
			bool foundFlg2 = false;
			Servent *s = servMgr->servents;
			while (s)
			{
				if (s->type == Servent::T_COUT && s->status == Servent::S_CONNECTED)
				{
					foundFlg = true;

					// ChannelData�����܂ŒT��
					ChannelData *prev = NULL;
					cd = channelDataTop;
					while (cd)
					{
						if (cd->type == Servent::T_COUT && cd->servent_id == s->servent_id)
						{
							foundFlg2 = true;
							cd->setEnableFlg(true);
							break;
						}
						prev = cd;
						cd = cd->getNextData();
					}
					cd = prev;

					if (foundFlg2)
						break;

					// �m�[�h�ǉ�
					if (channelDataTop)
					{
						// channelData����łȂ��Bcd�͂����Ń��X�g�������w���Ă�i�͂��j
						cd->setNextData(::new ChannelData());
						cd = cd->getNextData();
						memset(cd, 0, sizeof(cd));
						cd->setNextData(NULL);
					} else
					{
						// channelData����
						channelDataTop = ::new ChannelData();
						channelDataTop->setNextData(NULL);
						cd = channelDataTop;
					}

					// �f�[�^�ݒ�
					cd->type = s->type;
					cd->servent_id = s->servent_id;
					cd->setEnableFlg(true);
				}

				s = s->next;
			}

			// COUT���؂�Ă���폜
			if (!foundFlg)
			{
				cd = channelDataTop;
				ChannelData *prev = NULL;
				while (cd)
				{
					// COUT�̏����폜
					if (cd->type == Servent::T_COUT)
					{
						// �擪
						if (!prev)
						{
							channelDataTop = cd->getNextData();
						} else
						{
							prev->setNextData(cd->getNextData());
						}
						//::delete cd;
					}

					prev = cd;
					cd = cd->getNextData();
				}
			}
		}
#endif

		// �`�����l�����Ȃ��Ȃ��Ă���ꍇ�̏���
		cd = channelDataTop;
		ChannelData *prev = NULL; 
		while(cd){
			// �f�[�^���X�V���Ȃ�������
			if (cd->getEnableFlg() == FALSE){
				// �`�����l�����Ȃ��Ȃ��Ă���̂ō폜
				ChannelData *next;
				next = cd->getNextData();
				if (!prev){
					// �擪�̃f�[�^���폜
					// �������������[�N������ by ����[
					channelDataTop = next;
				} else {
					// �r���̃f�[�^���폜
					prev->setNextData(next);
				}
				// ���̃f�[�^��
				cd = next;
			} else {
				// �f�[�^�X�V�ρF���̃f�[�^��
				prev = cd;
				cd = cd->getNextData();
			}
		}

		Servent *s = servMgr->servents;
		while(s){
			// ������
			ChanHitList *chl;
			bool infoFlg = false;
			bool relay = true;
			bool firewalled = false;
			unsigned int numRelays = 0;
			int vp_ver = 0;
			char ver_ex_prefix[2] = {' ', ' '};
			int ver_ex_number = 0;
			// �����z�X�g���`�F�b�N
			unsigned int totalRelays = 0;
			unsigned int totalListeners = 0;

			ChanHit hitData;
			// ��M����
			if ((s->type == Servent::T_RELAY) && (s->status == Servent::S_CONNECTED)){
				// �z�X�g��񃍃b�N
				chanMgr->hitlistlock.on();
				// �����z�X�g����M���Ă���`�����l���̃z�X�g�����擾
				chl = chanMgr->findHitListByID(s->chanID);
				// �`�����l���̃z�X�g��񂪂��邩
				if (chl){
					// �`�����l���̃z�X�g��񂪂���ꍇ
					ChanHit *hit = chl->hit;
					//�@�`�����l���̃z�X�g����S��������
					while(hit){
						// ID���������̂ł����
						if (hit->servent_id == s->servent_id){
							// �g�[�^�������[�ƃg�[�^�����X�i�[�����Z
							totalRelays += hit->numRelays;
							totalListeners += hit->numListeners;
							// �����ł����
							if (hit->numHops == 1){
								// ������U�ۑ�
								infoFlg = true;
								hitData.relay = hit->relay;
								hitData.firewalled = hit->firewalled;
								hitData.numRelays = hit->numRelays;
								hitData.version_vp = hit->version_vp;
								hitData.version_ex_prefix[0] = hit->version_ex_prefix[0];
								hitData.version_ex_prefix[1] = hit->version_ex_prefix[1];
								hitData.version_ex_number = hit->version_ex_number;
							}
						}
						// �����`�F�b�N
						hit = hit->next;
					}
				}

				// �`�����l���f�[�^����Servent������
				bool bFoundFlg = FALSE;
				cd = channelDataTop;
				while(cd){
					ServentData *sv = cd->findServentData(s->servent_id);
					// ServentData�������
					if (sv && cd->getChannelId() == s->channel_id){
						// �f�[�^�ݒ�
						sv->setData(s, &hitData, totalListeners, totalRelays, infoFlg);
						sv->setEnableFlg(TRUE);
						bFoundFlg = TRUE;
						break;
					}
					cd = cd->getNextData();
				}
				// ServentData��������Ȃ������ꍇ
				if (!bFoundFlg){
					// �`�����l���f�[�^��T��
					cd = channelDataTop;
					while(cd){
						// �`�����l��ID��������
						if (cd->getChannelId() == s->channel_id){
							// �f�[�^�ݒ�
							ServentData *sv = ::new ServentData();
							sv->setData(s, &hitData, totalListeners, totalRelays, infoFlg);
							sv->setEnableFlg(TRUE);
							// �`�����l���f�[�^��ServentData�ǉ�
							cd->addServentData(sv);
							// �z�X�g�����擾����
							IdData *id = ::new IdData(cd->getChannelId(), sv->getServentId(), sv->getHost().ip);
							ThreadInfo *t;
							t = ::new ThreadInfo();
							t->func = GetHostName;
							t->data = (void*)id;
							sys->startThread(t);
							LOG_DEBUG("resolving thread was started(%d)", id->getServentId());
							// ���[�v�I��
							break;
						}
						// ���̃f�[�^��
						cd = cd->getNextData();
					}
				}
				// �z�X�g���A�����b�N
				chanMgr->hitlistlock.off();
			}
			s = s->next;
		}

		// �X�V���Ă��Ȃ�ServentData���폜
		cd = channelDataTop;
		while(cd){
			cd->deleteDisableServents();
			cd = cd->getNextData();
		}

		// �`�����l���f�[�^�A�����b�N
		ChannelDataLock.off();

		// �`��X�V
		if (guiWnd){
			MakeBack(guiWnd);
		}

		// 0.1�b�~10��1�b�҂�
		for(i=0; i<2; i++)
		{
			if (!thread->active)
				break;
			sys->sleep(100);
		}
	}

	// set GUI thread status to terminated
	thread->finish = true;

	return 0;
}

ChannelData *findChannelData(int channel_id){
	ChannelData *cd = channelDataTop;

	while(cd){
		if (cd->getChannelId() == channel_id){
			return cd;
		}
		cd = cd->getNextData();
	}

	return NULL;
}


void PopupChannelMenu(int channel_id){
	POINT pos;
	MENUITEMINFO info, separator;
	HMENU hMenu;
	DWORD dwID;

	hMenu = CreatePopupMenu();

	memset(&separator, 0, sizeof(MENUITEMINFO));
	separator.cbSize = sizeof(MENUITEMINFO);
	separator.fMask = MIIM_ID | MIIM_TYPE;
	separator.fType = MFT_SEPARATOR;
	separator.wID = 8000;

	memset(&info, 0, sizeof(MENUITEMINFO));
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_ID | MIIM_TYPE;
	info.fType = MFT_STRING;

	ChannelData *cd = findChannelData(channel_id);

	if (cd == NULL){
		return;
	}

	info.wID = 1001;
	info.dwTypeData = "�ؒf";
	InsertMenuItem(hMenu, -1, true, &info);

	InsertMenuItem(hMenu, -1, true, &separator);

	info.wID = 1000;
	info.dwTypeData = "�Đ�";
	InsertMenuItem(hMenu, -1, true, &info);

	InsertMenuItem(hMenu, -1, true, &separator);

	info.wID = 1002;
	info.dwTypeData = "�Đڑ�";
	InsertMenuItem(hMenu, -1, true, &info);

	info.wID = 1003;
	info.dwTypeData = "�L�[�v";
	InsertMenuItem(hMenu, -1, true, &info);

	InsertMenuItem(hMenu, -1, true, &separator);

	if (!cd->getOpenFlg()){
		info.wID = 1004;
		info.dwTypeData = "�����\��";
		InsertMenuItem(hMenu, -1, true, &info);
	} else {
		info.wID = 1005;
		info.dwTypeData = "�����B��";
		InsertMenuItem(hMenu, -1, true, &info);
	}

	GetCursorPos(&pos);
	dwID = TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_RETURNCMD, pos.x, pos.y, 0, guiWnd, NULL);

	DestroyMenu(hMenu);

	cd = findChannelData(channel_id);

	if (cd == NULL){
		return;
	}

	Channel *c = chanMgr->findChannelByChannelID(channel_id);

	if (c == NULL){
		return;
	}

	switch(dwID){
		case 1000:	// �Đ�
			chanMgr->playChannel(c->info);
			break;

		case 1001:	// �ؒf
			// bump���͐ؒf���Ȃ�
			if (!c->bumped)
			{
				c->thread.active = false;
				c->thread.finish = true;
			}
			break;

		case 1002:	// �Đڑ�
			// ��������M���ł���Ίm�F���b�Z�[�W�\��
			if (cd->isTracker() && cd->getStatus() == Channel::S_RECEIVING)
			{
				int id;
				id = MessageBox(guiWnd,
					"�����ł����Đڑ����܂����H",
					"�����x��",
					MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2);
				if (id != IDYES)
					break;
			}

			c->bump = true;
			break;

		case 1003:	// �L�[�v
			if (!c->stayConnected){
				c->stayConnected  = true;
			} else {
				c->stayConnected = false;
			}
			break;

		case 1004:	// �����\��
			cd->setOpenFlg(TRUE);
			MakeBack(guiWnd);
			break;

		case 1005:	// �����B��
			cd->setOpenFlg(FALSE);
			MakeBack(guiWnd);
			break;
	}
}

void PopupServentMenu(int servent_id){
	POINT pos;
	MENUITEMINFO info, separator;
	HMENU hMenu;
	DWORD dwID;

	hMenu = CreatePopupMenu();

	memset(&separator, 0, sizeof(MENUITEMINFO));
	separator.cbSize = sizeof(MENUITEMINFO);
	separator.fMask = MIIM_ID | MIIM_TYPE;
	separator.fType = MFT_SEPARATOR;
	separator.wID = 8000;

	memset(&info, 0, sizeof(MENUITEMINFO));
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_ID | MIIM_TYPE;
	info.fType = MFT_STRING;

	ServentData *sd = NULL;
	ChannelData *cd = channelDataTop;
	while(cd){
		// COUT
		if (cd->type == Servent::T_COUT
			&& cd->servent_id == servent_id)
			break;

		sd = cd->findServentData(servent_id);
		if (sd){
			break;
		}
		cd = cd->getNextData();
	}

	if (cd == NULL || sd == NULL
		&& cd->type != Servent::T_COUT) // COUT
	{
		return;
	}

	info.wID = 1001;
	info.dwTypeData = "�ؒf";
	InsertMenuItem(hMenu, -1, true, &info);

//	InsertMenuItem(hMenu, -1, true, &separator);

	GetCursorPos(&pos);
	dwID = TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_RETURNCMD, pos.x, pos.y, 0, guiWnd, NULL);

	DestroyMenu(hMenu);

	cd = channelDataTop;
	while(cd){
		// COUT
		if (cd->type == Servent::T_COUT
			&& cd->servent_id == servent_id)
			break;

		sd = cd->findServentData(servent_id);
		if (sd){
			break;
		}
		cd = cd->getNextData();
	}

	if (cd == NULL || sd == NULL
		&& cd->type != Servent::T_COUT) // COUT
	{
		return;
	}

	Servent *s = servMgr->findServentByServentID(servent_id);

	if (s == NULL){
		return;
	}

	switch(dwID){
		case 1001:	// �ؒf
			s->thread.active = false;

			// COUT�ؒf
			if (s->type == Servent::T_COUT)
				s->thread.finish = true;

			break;

	}
}

void PopupOtherMenu(){
	POINT pos;
	MENUITEMINFO info, separator;
	HMENU hMenu;
	DWORD dwID;

	hMenu = CreatePopupMenu();

	memset(&separator, 0, sizeof(MENUITEMINFO));
	separator.cbSize = sizeof(MENUITEMINFO);
	separator.fMask = MIIM_ID | MIIM_TYPE;
	separator.fType = MFT_SEPARATOR;
	separator.wID = 8000;

	memset(&info, 0, sizeof(MENUITEMINFO));
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_ID | MIIM_TYPE;
	info.fType = MFT_STRING;

	info.wID = 1107;
	info.dwTypeData = "�񃊃��[���̃`�����l�����폜";
	InsertMenuItem(hMenu, -1, true, &info);

	InsertMenuItem(hMenu, -1, true, &separator);

	if (!gbDispTop){
		info.wID = 1101;
		info.dwTypeData = "�őO�ʕ\��";
		InsertMenuItem(hMenu, -1, true, &info);
	} else {
		info.wID = 1102;
		info.dwTypeData = "�őO�ʉ���";
		InsertMenuItem(hMenu, -1, true, &info);
	}

	InsertMenuItem(hMenu, -1, true, &separator);

	if (!gbAllOpen){
		info.wID = 1103;
		info.dwTypeData = "�S�����W�J";
		InsertMenuItem(hMenu, -1, true, &info);
	} else {
		info.wID = 1104;
		info.dwTypeData = "�S�����B��";
		InsertMenuItem(hMenu, -1, true, &info);
	}

	InsertMenuItem(hMenu, -1, true, &separator);

	if (!servMgr->autoServe){
		info.wID = 1105;
		info.dwTypeData = "�L��";
		InsertMenuItem(hMenu, -1, true, &info);
	} else {
		info.wID = 1106;
		info.dwTypeData = "����";
		InsertMenuItem(hMenu, -1, true, &info);
	}

	GetCursorPos(&pos);
	dwID = TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_RETURNCMD, pos.x, pos.y, 0, guiWnd, NULL);

	DestroyMenu(hMenu);

	ChannelData *cd = channelDataTop;

	switch(dwID){
		case 1101:	// �őO�ʕ\��
			gbDispTop = true;
			::SetWindowPos(guiWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			break;

		case 1102:	// �őO�ʉ���
			gbDispTop = false;
			::SetWindowPos(guiWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			break;

		case 1103:	// �S�����W�J
			gbAllOpen = true;
			while(cd){
				cd->setOpenFlg(true);
				cd = cd->getNextData();
			}
			break;

		case 1104:	// �S�����B��
			gbAllOpen = false;
			while(cd){
				cd->setOpenFlg(false);
				cd = cd->getNextData();
			}
			break;

		case 1105:	// �L��
			servMgr->autoServe = true;
			break;

		case 1106:	// ����
			servMgr->autoServe = false;
			break;

		case 1107:  // �񃊃��[���̃`�����l������S�č폜
			{
				LOG_DEBUG("Start cleaning up unused channels");
				while (cd)
				{
					if (cd->getStatus() == Channel::S_NOTFOUND
						|| cd->getStatus() == Channel::S_IDLE)
					{
						Channel *c = chanMgr->findChannelByChannelID(cd->getChannelId());

						if (c && !c->bumped)
						{
							c->thread.active = false;
							c->thread.finish = true;
						}
					}

					cd = cd->getNextData();
				}
				LOG_DEBUG("Finish a cleanup of unused channels");
			}
			break;
	}
}

void WmCreateProc(HWND hwnd){
	// �����őO�ʋ@�\
	if (servMgr->topmostGui)
	{
		::gbDispTop = true;
	}

	if (backImage){
		::delete backImage;
	}
	_bstr_t bstr("back.jpg");
	backImage = ::new Image(bstr);

	MakeBack(hwnd, 800, 600);

	guiThread.func = GUIDataUpdate;
	if (!sys->startThread(&guiThread)){
		MessageBox(hwnd,"Unable to start GUI","PeerCast",MB_OK|MB_ICONERROR);
		PostMessage(hwnd,WM_DESTROY,0,0);
	}
	if (guiFlg){
		SetWindowPlacement(hwnd, &winPlace);
	}

	if (img_idle){
		::delete img_idle;
		::delete img_connect;
		::delete img_conn_ok;
		::delete img_conn_full;
		::delete img_conn_over;
		::delete img_conn_ok_skip;
		::delete img_conn_full_skip;
		::delete img_conn_over_skip;
		::delete img_error;
		::delete img_broad_ok;
		::delete img_broad_full;
	}
	bstr = L"ST_IDLE.bmp";
	img_idle = ::new Image(bstr);
	bstr = L"ST_CONNECT.bmp";
	img_connect = ::new Image(bstr);
	bstr = L"ST_CONN_OK.bmp";
	img_conn_ok = ::new Image(bstr);
	bstr = L"ST_CONN_FULL.bmp";
	img_conn_full = ::new Image(bstr);
	bstr = L"ST_CONN_OVER.bmp";
	img_conn_over = ::new Image(bstr);
	bstr = L"ST_CONN_OK_SKIP.bmp";
	img_conn_ok_skip = ::new Image(bstr);
	bstr = L"ST_CONN_FULL_SKIP.bmp";
	img_conn_full_skip = ::new Image(bstr);
	bstr = L"ST_CONN_OVER_SKIP.bmp";
	img_conn_over_skip = ::new Image(bstr);
	bstr = L"ST_ERROR.bmp";
	img_error = ::new Image(bstr);
	bstr = L"ST_BROAD_OK.bmp";
	img_broad_ok = ::new Image(bstr);
	bstr = L"ST_BROAD_FULL.bmp";
	img_broad_full = ::new Image(bstr);

	// jumplist
	if (jumpListEnabled)
	{
	}
}

void WmPaintProc(HWND hwnd){
	HDC hdc;
	PAINTSTRUCT paint;

	if (backGra){
		MakeBackLock.on();
		hdc = BeginPaint(hwnd, &paint);
		RECT *rcRect;	// �`��͈�
		rcRect = &(paint.rcPaint);
		LONG width = rcRect->right - rcRect->left + 1;
		LONG height = rcRect->bottom - rcRect->top + 1;

		Graphics g2(hdc);
		Rect r(rcRect->left, rcRect->top, width, height);
		g2.DrawImage(backBmp,r, rcRect->left, rcRect->top, width, height, UnitPixel);
		EndPaint(hwnd, &paint);
		MakeBackLock.off();
	}
}

void WmSizeProc(HWND hwnd, LPARAM lParam){
	UINT width = LOWORD(lParam);
	UINT height = HIWORD(lParam);

	MakeBack(hwnd, width, height);

}

void WmLButtonDownProc(HWND hwnd, LPARAM lParam){
	ChannelData *cd;
	bool changeFlg = FALSE;

	ChannelDataLock.on();
	cd = channelDataTop;
	while(cd){
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (cd->checkDown(LOWORD(lParam), HIWORD(lParam))){
			if (!(cd->isSelected())){
				changeFlg = TRUE;
			}
			cd->setSelected(TRUE);
		} else {
			if (cd->isSelected()){
				changeFlg = TRUE;
			}
			cd->setSelected(FALSE);
		}
		int sx = cd->getPosX() + cd->getWidth();
		int sy = cd->getPosY();
		int index = 0;
		ServentData *sd = cd->getServentDataTop();
		while(sd){
			if (	(	(!cd->getOpenFlg())
					&&	(sx + index*14 < x)
					&&	(x < sx + (index+1)*14)
					&&	(sy < y)
					&&	(y < sy + 14)	)
				||	sd->checkDown(LOWORD(lParam), HIWORD(lParam))
			){
				if (!sd->getSelected()){
					changeFlg = TRUE;
				}
				sd->setSelected(TRUE);
			} else {
				if (sd->getSelected()){
					changeFlg = TRUE;
				}
				sd->setSelected(FALSE);
			}
			sd = sd->getNextData();
			index++;
		}
		cd = cd->getNextData();
	}
	ChannelDataLock.off();
	if (changeFlg){
		MakeBack(hwnd);
	}
}

void WmLButtonDblclkProc(HWND hwnd, LPARAM lParam){
	ChannelData *cd;
	bool changeFlg = FALSE;

	ChannelDataLock.on();
	cd = channelDataTop;
	while(cd){
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (cd->checkDown(LOWORD(lParam), HIWORD(lParam))){
			if (!(cd->isSelected())){
				changeFlg = TRUE;
			}
			if (!(cd->getOpenFlg())){
				changeFlg = TRUE;
				cd->setOpenFlg(TRUE);
			} else {
				changeFlg = TRUE;
				cd->setOpenFlg(FALSE);
			}
			cd->setSelected(TRUE);
		} else {
			if (cd->isSelected()){
				changeFlg = TRUE;
			}
			cd->setSelected(FALSE);
		}
/*		int sx = cd->getPosX() + cd->getWidth();
		int sy = cd->getPosY();
		int index = 0;
		ServentData *sd = cd->getServentDataTop();
		while(sd){
			if (	(	(!cd->getOpenFlg())
					&&	(sx + index*14 < x)
					&&	(x < sx + (index+1)*14)
					&&	(sy < y)
					&&	(y < sy + 14)	)
				||	sd->checkDown(LOWORD(lParam), HIWORD(lParam))
			){
				if (!sd->getSelected()){
					changeFlg = TRUE;
				}
				sd->setSelected(TRUE);
			} else {
				if (sd->getSelected()){
					changeFlg = TRUE;
				}
				sd->setSelected(FALSE);
			}
			sd = sd->getNextData();
			index++;
		}*/
		cd = cd->getNextData();
	}
	ChannelDataLock.off();
	if (changeFlg){
		MakeBack(hwnd);
	}
}

void WmRButtonDownProc(HWND hwnd, LPARAM lParam){
	ChannelData *cd;
	bool changeFlg = FALSE;
	bool channel_selected = FALSE;
	bool servent_selected = FALSE;
	int channel_id = 0;
	int servent_id = 0;

	cd = channelDataTop;
	while(cd){
		if (cd->checkDown(LOWORD(lParam), HIWORD(lParam))){
			if (!(cd->isSelected())){
				changeFlg = TRUE;
			}
			cd->setSelected(TRUE);
			channel_id = cd->getChannelId();
			channel_selected = TRUE;

			// COUT����
			if (cd->type == Servent::T_COUT)
			{
				channel_selected = FALSE;
				servent_selected = TRUE;
				servent_id = cd->servent_id;
			}
		} else {
			if (cd->isSelected()){
				changeFlg = TRUE;
			}
			cd->setSelected(FALSE);
		}
		ServentData *sd = cd->getServentDataTop();
		while(sd){
			if (sd->checkDown(LOWORD(lParam), HIWORD(lParam))){
				if (!sd->getSelected()){
					changeFlg = TRUE;
				}
				sd->setSelected(TRUE);
				servent_id = sd->getServentId();
				servent_selected = TRUE;
			} else {
				if (sd->getSelected()){
					changeFlg = TRUE;
				}
				sd->setSelected(FALSE);
			}
			sd = sd->getNextData();
		}
		cd = cd->getNextData();
	}
	if (changeFlg){
		MakeBack(hwnd);
	}

	if (channel_selected){
		PopupChannelMenu(channel_id);
	} else if (servent_selected){
		PopupServentMenu(servent_id);
	} else {
		PopupOtherMenu();
	}
}

LRESULT CALLBACK GUIProc (HWND hwnd, UINT message,
                                 WPARAM wParam, LPARAM lParam)
{
	switch(message){
		case WM_CREATE:		// �E�B���h�E�쐬
			WmCreateProc(hwnd);
			break;

		case WM_PAINT:		// �`��
			WmPaintProc(hwnd);
			break;

		case WM_SIZE:		// �T�C�Y�ύX
			WmSizeProc(hwnd,lParam);
			break;

		case WM_LBUTTONDOWN:	// ���{�^������
			WmLButtonDownProc(hwnd,lParam);
			break;

		case WM_RBUTTONDOWN:	// �E�{�^������
			WmRButtonDownProc(hwnd,lParam);
			break;

		case WM_LBUTTONDBLCLK:		// ���_�u���N���b�N
			WmLButtonDblclkProc(hwnd,lParam);
			break;

		case WM_ERASEBKGND:	// �w�i����
			return TRUE;	// �w�i�͏����Ȃ�

		case WM_CLOSE:
			//if (backImage){
			//	::delete backImage;
			//	backImage = NULL;
			//}
			GetWindowPlacement(hwnd, &winPlace);
			guiFlg = true;
			DestroyWindow( hwnd );
			break;
		case WM_DESTROY:
			GetWindowPlacement(hwnd, &winPlace);
			guiFlg = true;
			guiThread.active = false;

			// wait until GUI thread terminated,
			// and then dispose background image.
			while (1)
			{
				if (guiThread.finish)
					break;
			}
			if (backImage)
			{
				::delete backImage;
				backImage = NULL;
			}

			guiWnd = NULL;
			break;

		default:
			return (DefWindowProc(hwnd, message, wParam, lParam));
	}

	return 0;
}
