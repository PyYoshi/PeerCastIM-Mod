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

	// 速度表示部の背景を白くする
	SolidBrush b(Color(180,255,255,255));
	backGra->FillRectangle(&b, posX, posY, 200, 14);
	// フォント設定
	Font font(L"ＭＳ Ｐゴシック",10);
	// 文字色
	SolidBrush strBrush(Color::Black);
	// 文字列作成
	char tmp[256];
	sprintf(tmp, "R:%.1fkbps S:%.1fkbps", 
		BYTES_TO_KBPS(stats.getPerSecond(Stats::BYTESIN)-stats.getPerSecond(Stats::LOCALBYTESIN)),
		BYTES_TO_KBPS(stats.getPerSecond(Stats::BYTESOUT)-stats.getPerSecond(Stats::LOCALBYTESOUT)));
	_bstr_t bstr(tmp);
	// 文字表示範囲指定
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	RectF r((REAL)posX, (REAL)posY, (REAL)200, (REAL)14);
	// 文字描画
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

	// 全て白で塗りつぶし
	SolidBrush b(Color(255,255,255,255));
	backGra->FillRectangle(&b, 0, 0, x, y);

	backWidth = backImage->GetWidth();
	backHeight = backImage->GetHeight();

	// 背景画像を描画
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

	// 速度描画
	drawSpeed(backGra, winWidth-205, 5);

	// チャンネル情報を描画
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

	// 位置を保存
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

	// チャンネル表示部の背景を塗る
	if (isSelected()){
		// 選択中
		SolidBrush b(Color(160,49,106,197));
		g->FillRectangle(&b, x, y, w, 14);
	} else {
		// 非選択
		SolidBrush b(Color(160,255,255,255));
		g->FillRectangle(&b, x, y, w, 14);
	}

	// ステータス表示
	Gdiplus::Image *img = NULL;
	unsigned int nowTime = sys->getTime();
	if (this->type != Servent::T_COUT)
	{
		// COUT以外
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
			// bump時にエラーが表示されるのを防止
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
		// COUT用
		img = img_broad_ok;
	}

	// 描画基点
	PointF origin(xx, yy);
	// ステータス表示位置
	Rect img_rect((INT)origin.X, (INT)origin.Y + 1, img ? img->GetWidth() : 12, 12);
	// ステータス描画
	ImageAttributes att;
//	att.SetColorKey(Color::White, Color::White, ColorAdjustTypeBitmap);
	g->DrawImage(img, img_rect, 0, 0, img_rect.Width, 12, UnitPixel, &att);
	// 次の基点
	origin.X += img_rect.Width;

	// フォント設定
	Gdiplus::Font font(L"ＭＳ Ｐゴシック",10);
	// 文字色
	SolidBrush *strBrush;
	if (servMgr->getFirewall() == ServMgr::FW_ON){
		strBrush = ::new SolidBrush(Color::Red);
	} else if (isTracker() && (getStatus() == Channel::S_RECEIVING)){
		strBrush = ::new SolidBrush(Color::Green);
	} else {
		if (isSelected()){
			// 選択中
			strBrush = ::new SolidBrush(Color::White);
		} else {
			// 非選択
			strBrush = ::new SolidBrush(Color::Black);
		}
	}

	if (this->type != Servent::T_COUT)
	{
		// COUT以外

		// チャンネル名表示
		g->SetTextRenderingHint(TextRenderingHintAntiAlias);
		_bstr_t bstr1(getName());
		// 文字描画範囲指定
		RectF r1(origin.X, origin.Y, 120.0f, 13.0f);
		StringFormat format;
		format.SetAlignment(StringAlignmentNear);
		g->DrawString(bstr1, -1, &font, r1, &format, strBrush);
		// 次の基点
		origin.X += r1.Width;

		//// 上流IP/リスナー数/リレー数表示
		//// NOTE:
		////    ぴあかすの動作勉強用。リリースビルドでは元のコードを使用の事。
		////    文字表示範囲は幅220ぐらいでおｋ
		//char tmp[512]; // 表示用バッファ
		//char hostip[256]; // IPアドレスバッファ
		//chDisp.uphost.toStr(hostip); // 上流IP
		//sprintf(tmp, "%d/%d - [%d/%d] - %s",
		//	getTotalListeners(),
		//	getTotalRelays(),
		//	getLocalListeners(),
		//	getLocalRelays(),
		//	hostip
		//	);

		// リスナー数/リレー数表示
		char tmp[256];
		sprintf(tmp, "%d/%d - [%d/%d]", getTotalListeners(), getTotalRelays(), getLocalListeners(), getLocalRelays());
		_bstr_t bstr2(tmp);
		// 文字表示範囲指定
		RectF r2(origin.X, origin.Y, 100.0f, 13.0f);
		format.SetAlignment(StringAlignmentCenter);
		g->DrawString(bstr2, -1, &font, r2, &format, strBrush);
		// 次の基点
		origin.X += r2.Width;

		// bps表示
		Font *f;
		if (isStayConnected()){
			f = ::new Font(L"Arial", 9.0f, FontStyleItalic|FontStyleBold, UnitPoint);
		} else {
			f = ::new Font(L"Arial", 9.0f);
		}
		sprintf(tmp, "%dkbps", getBitRate());
		_bstr_t bstr3(tmp);
		format.SetAlignment(StringAlignmentFar);
		// 文字表示範囲指定
		RectF r3(origin.X, origin.Y, 80.0f, 13.0f);
		g->DrawString(bstr3, -1, f, r3, &format, strBrush);
		// フォント開放
		::delete f;

		// 次の基点
		origin.X += r3.Width;

		// ブラシ削除
		::delete strBrush;


		// Servent表示
		if (!openFlg){
			int count = getServentCount();
			// Servent表示部の背景を白にする
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
						// リレーＯＫ
						serventBrush = ::new SolidBrush(Color::Green);
					} else {
						// リレー不可
						if (hit->numRelays){
							// リレー一杯
							serventBrush = ::new SolidBrush(Color::Blue);
						} else {
							// リレーなし
							serventBrush = ::new SolidBrush(Color::Purple);
						}
					}
				} else {
					// 情報なし
					serventBrush = ::new SolidBrush(Color::Black);
				}
				// 四角描画
				backGra->FillRectangle(serventBrush, (INT)origin.X + index*14 + 1, (INT)origin.Y + 1, 12, 12);

				::delete serventBrush;
				sd = sd->getNextData();
				index++;
			}
		}

		// 次の基点
		origin.Y += 15;

		// サイズを保存
		setWidth((int)origin.X - posX);
		setHeight((int)origin.Y - posY);

		// ServentData表示
		sd = serventDataTop;
		while(sd){
			if (openFlg || sd->getSelected()){
				sd->drawServent(g, (INT)x+12, (INT)origin.Y);
				// 次の基点
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
	// 範囲内チェック
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

	// 位置を保存
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

	// 描画基点
	PointF origin(xx, yy);

	// フォント設定
	Font font(L"ＭＳ Ｐゴシック",9);
	// 文字色
	SolidBrush *strBrush;
	if (chanHit.firewalled){
		strBrush = ::new SolidBrush(Color::Red);
	} else {
		if (getSelected()){
			// 選択中
			strBrush = ::new SolidBrush(Color::White);
		} else {
			// 非選択
			strBrush = ::new SolidBrush(Color::Black);
		}
	}
	// ServantData表示
	g->SetTextRenderingHint(TextRenderingHintAntiAlias);
	// 文字列作成
	char tmp[256];
	char host1[256];
	host.toStr(host1);

	if (infoFlg){
		if (chanHit.version_ex_number){
			// 拡張バージョン
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

	// ステータス表示
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

	// 文字描画範囲指定
	RectF r1(origin.X + img->GetWidth() + 2, origin.Y, 800.0f, 13.0f);
	RectF r2;
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	g->MeasureString(bstr1, -1, &font, r1, &format, &r2);

	w = (INT)r2.Width + img->GetWidth() + 2;
	// ServentData表示部の背景を塗る
	if (getSelected()){
		// 選択中
		SolidBrush b(Color(160,49,106,197));
		g->FillRectangle(&b, x, y, w, 13);
	} else {
		// 非選択
		SolidBrush b(Color(160,200,200,200));
		g->FillRectangle(&b, x, y, w, 13);
	}

	// ステータス表示位置
	Rect img_rect((INT)origin.X, (INT)origin.Y+1, img ? img->GetWidth() : 12, 12);
	// ステータス描画
	ImageAttributes att;
//	att.SetColorKey(Color::White, Color::White, ColorAdjustTypeBitmap);
	g->DrawImage(img, img_rect, 0, 0, img_rect.Width, 12, UnitPixel, &att);
	// 次の基点
	origin.X += 12;

	g->DrawString(bstr1, -1, &font, r2, &format, strBrush);
	// 次の基点
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
		// チャンネルデータロック
		ChannelDataLock.on();
		// チャンネルデータの更新フラグを全てFALSEにする
		ChannelData *cd = channelDataTop;
		while(cd){
			// Serventの更新フラグをFALSEにする
			ServentData *sv = cd->getServentDataTop();
			while(sv){
				sv->setEnableFlg(FALSE);
				sv = sv->getNextData();
			}
			cd->setEnableFlg(FALSE);
			cd = cd->getNextData();
		}

		Channel *c = chanMgr->channel;
		// 現在存在するチャンネル分ループ
		while(c){
			// 既にチャンネルデータを持っているか
			cd = channelDataTop;
			// 発見フラグFALSE
			bool bFoundFlg = FALSE;
			while(cd){
				if (cd->getChannelId() == c->channel_id){
					//既にチャンネルデータがあるので、そのまま更新
					cd->setData(c);
					// 更新フラグTRUE
					cd->setEnableFlg(TRUE);
					// 発見フラグTRUE
					bFoundFlg = TRUE;
					// ループ離脱
					break;
				}
				// 見つからなかった場合、次のデータをチェック
				cd = cd->getNextData();
			}

			// 新しいチャンネルの場合、新規データ作成
			if (!bFoundFlg){
				// 新規データ作成
				cd = ::new ChannelData();
				// データ更新
				cd->setData(c);
				// 更新フラグTRUE
				cd->setEnableFlg(TRUE);

				// 新規データをリストの先頭に入れる
				cd->setNextData(channelDataTop);
				channelDataTop = cd;
			}
			// 次のチャンネルを取得
			c = c->next;
		}

#if 1
		// COUTを検索
		{
			bool foundFlg = false;
			bool foundFlg2 = false;
			Servent *s = servMgr->servents;
			while (s)
			{
				if (s->type == Servent::T_COUT && s->status == Servent::S_CONNECTED)
				{
					foundFlg = true;

					// ChannelData末尾まで探索
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

					// ノード追加
					if (channelDataTop)
					{
						// channelDataが空でない。cdはここでリスト末尾を指してる（はず）
						cd->setNextData(::new ChannelData());
						cd = cd->getNextData();
						memset(cd, 0, sizeof(cd));
						cd->setNextData(NULL);
					} else
					{
						// channelDataが空
						channelDataTop = ::new ChannelData();
						channelDataTop->setNextData(NULL);
						cd = channelDataTop;
					}

					// データ設定
					cd->type = s->type;
					cd->servent_id = s->servent_id;
					cd->setEnableFlg(true);
				}

				s = s->next;
			}

			// COUTが切れてたら削除
			if (!foundFlg)
			{
				cd = channelDataTop;
				ChannelData *prev = NULL;
				while (cd)
				{
					// COUTの情報を削除
					if (cd->type == Servent::T_COUT)
					{
						// 先頭
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

		// チャンネルがなくなっている場合の処理
		cd = channelDataTop;
		ChannelData *prev = NULL; 
		while(cd){
			// データを更新しなかったか
			if (cd->getEnableFlg() == FALSE){
				// チャンネルがなくなっているので削除
				ChannelData *next;
				next = cd->getNextData();
				if (!prev){
					// 先頭のデータを削除
					// ここメモリリークしそう by えるー
					channelDataTop = next;
				} else {
					// 途中のデータを削除
					prev->setNextData(next);
				}
				// 次のデータへ
				cd = next;
			} else {
				// データ更新済：次のデータへ
				prev = cd;
				cd = cd->getNextData();
			}
		}

		Servent *s = servMgr->servents;
		while(s){
			// 初期化
			ChanHitList *chl;
			bool infoFlg = false;
			bool relay = true;
			bool firewalled = false;
			unsigned int numRelays = 0;
			int vp_ver = 0;
			char ver_ex_prefix[2] = {' ', ' '};
			int ver_ex_number = 0;
			// 直下ホスト情報チェック
			unsigned int totalRelays = 0;
			unsigned int totalListeners = 0;

			ChanHit hitData;
			// 受信中か
			if ((s->type == Servent::T_RELAY) && (s->status == Servent::S_CONNECTED)){
				// ホスト情報ロック
				chanMgr->hitlistlock.on();
				// 直下ホストが受信しているチャンネルのホスト情報を取得
				chl = chanMgr->findHitListByID(s->chanID);
				// チャンネルのホスト情報があるか
				if (chl){
					// チャンネルのホスト情報がある場合
					ChanHit *hit = chl->hit;
					//　チャンネルのホスト情報を全走査して
					while(hit){
						// IDが同じものであれば
						if (hit->servent_id == s->servent_id){
							// トータルリレーとトータルリスナーを加算
							totalRelays += hit->numRelays;
							totalListeners += hit->numListeners;
							// 直下であれば
							if (hit->numHops == 1){
								// 情報を一旦保存
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
						// 次をチェック
						hit = hit->next;
					}
				}

				// チャンネルデータからServentを検索
				bool bFoundFlg = FALSE;
				cd = channelDataTop;
				while(cd){
					ServentData *sv = cd->findServentData(s->servent_id);
					// ServentDataがあれば
					if (sv && cd->getChannelId() == s->channel_id){
						// データ設定
						sv->setData(s, &hitData, totalListeners, totalRelays, infoFlg);
						sv->setEnableFlg(TRUE);
						bFoundFlg = TRUE;
						break;
					}
					cd = cd->getNextData();
				}
				// ServentDataが見つからなかった場合
				if (!bFoundFlg){
					// チャンネルデータを探す
					cd = channelDataTop;
					while(cd){
						// チャンネルIDが同じか
						if (cd->getChannelId() == s->channel_id){
							// データ設定
							ServentData *sv = ::new ServentData();
							sv->setData(s, &hitData, totalListeners, totalRelays, infoFlg);
							sv->setEnableFlg(TRUE);
							// チャンネルデータにServentData追加
							cd->addServentData(sv);
							// ホスト名を取得する
							IdData *id = ::new IdData(cd->getChannelId(), sv->getServentId(), sv->getHost().ip);
							ThreadInfo *t;
							t = ::new ThreadInfo();
							t->func = GetHostName;
							t->data = (void*)id;
							sys->startThread(t);
							LOG_DEBUG("resolving thread was started(%d)", id->getServentId());
							// ループ終了
							break;
						}
						// 次のデータへ
						cd = cd->getNextData();
					}
				}
				// ホスト情報アンロック
				chanMgr->hitlistlock.off();
			}
			s = s->next;
		}

		// 更新していないServentDataを削除
		cd = channelDataTop;
		while(cd){
			cd->deleteDisableServents();
			cd = cd->getNextData();
		}

		// チャンネルデータアンロック
		ChannelDataLock.off();

		// 描画更新
		if (guiWnd){
			MakeBack(guiWnd);
		}

		// 0.1秒×10で1秒待ち
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
	info.dwTypeData = "切断";
	InsertMenuItem(hMenu, -1, true, &info);

	InsertMenuItem(hMenu, -1, true, &separator);

	info.wID = 1000;
	info.dwTypeData = "再生";
	InsertMenuItem(hMenu, -1, true, &info);

	InsertMenuItem(hMenu, -1, true, &separator);

	info.wID = 1002;
	info.dwTypeData = "再接続";
	InsertMenuItem(hMenu, -1, true, &info);

	info.wID = 1003;
	info.dwTypeData = "キープ";
	InsertMenuItem(hMenu, -1, true, &info);

	InsertMenuItem(hMenu, -1, true, &separator);

	if (!cd->getOpenFlg()){
		info.wID = 1004;
		info.dwTypeData = "直下表示";
		InsertMenuItem(hMenu, -1, true, &info);
	} else {
		info.wID = 1005;
		info.dwTypeData = "直下隠蔽";
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
		case 1000:	// 再生
			chanMgr->playChannel(c->info);
			break;

		case 1001:	// 切断
			// bump中は切断しない
			if (!c->bumped)
			{
				c->thread.active = false;
				c->thread.finish = true;
			}
			break;

		case 1002:	// 再接続
			// 直下かつ受信中であれば確認メッセージ表示
			if (cd->isTracker() && cd->getStatus() == Channel::S_RECEIVING)
			{
				int id;
				id = MessageBox(guiWnd,
					"直下ですが再接続しますか？",
					"直下警告",
					MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2);
				if (id != IDYES)
					break;
			}

			c->bump = true;
			break;

		case 1003:	// キープ
			if (!c->stayConnected){
				c->stayConnected  = true;
			} else {
				c->stayConnected = false;
			}
			break;

		case 1004:	// 直下表示
			cd->setOpenFlg(TRUE);
			MakeBack(guiWnd);
			break;

		case 1005:	// 直下隠蔽
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
	info.dwTypeData = "切断";
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
		case 1001:	// 切断
			s->thread.active = false;

			// COUT切断
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
	info.dwTypeData = "非リレー中のチャンネルを削除";
	InsertMenuItem(hMenu, -1, true, &info);

	InsertMenuItem(hMenu, -1, true, &separator);

	if (!gbDispTop){
		info.wID = 1101;
		info.dwTypeData = "最前面表示";
		InsertMenuItem(hMenu, -1, true, &info);
	} else {
		info.wID = 1102;
		info.dwTypeData = "最前面解除";
		InsertMenuItem(hMenu, -1, true, &info);
	}

	InsertMenuItem(hMenu, -1, true, &separator);

	if (!gbAllOpen){
		info.wID = 1103;
		info.dwTypeData = "全直下展開";
		InsertMenuItem(hMenu, -1, true, &info);
	} else {
		info.wID = 1104;
		info.dwTypeData = "全直下隠蔽";
		InsertMenuItem(hMenu, -1, true, &info);
	}

	InsertMenuItem(hMenu, -1, true, &separator);

	if (!servMgr->autoServe){
		info.wID = 1105;
		info.dwTypeData = "有効";
		InsertMenuItem(hMenu, -1, true, &info);
	} else {
		info.wID = 1106;
		info.dwTypeData = "無効";
		InsertMenuItem(hMenu, -1, true, &info);
	}

	GetCursorPos(&pos);
	dwID = TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_RETURNCMD, pos.x, pos.y, 0, guiWnd, NULL);

	DestroyMenu(hMenu);

	ChannelData *cd = channelDataTop;

	switch(dwID){
		case 1101:	// 最前面表示
			gbDispTop = true;
			::SetWindowPos(guiWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			break;

		case 1102:	// 最前面解除
			gbDispTop = false;
			::SetWindowPos(guiWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			break;

		case 1103:	// 全直下展開
			gbAllOpen = true;
			while(cd){
				cd->setOpenFlg(true);
				cd = cd->getNextData();
			}
			break;

		case 1104:	// 全直下隠蔽
			gbAllOpen = false;
			while(cd){
				cd->setOpenFlg(false);
				cd = cd->getNextData();
			}
			break;

		case 1105:	// 有効
			servMgr->autoServe = true;
			break;

		case 1106:	// 無効
			servMgr->autoServe = false;
			break;

		case 1107:  // 非リレー中のチャンネル情報を全て削除
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
	// 自動最前面機能
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
		RECT *rcRect;	// 描画範囲
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

			// COUT識別
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
		case WM_CREATE:		// ウィンドウ作成
			WmCreateProc(hwnd);
			break;

		case WM_PAINT:		// 描画
			WmPaintProc(hwnd);
			break;

		case WM_SIZE:		// サイズ変更
			WmSizeProc(hwnd,lParam);
			break;

		case WM_LBUTTONDOWN:	// 左ボタン押す
			WmLButtonDownProc(hwnd,lParam);
			break;

		case WM_RBUTTONDOWN:	// 右ボタン押す
			WmRButtonDownProc(hwnd,lParam);
			break;

		case WM_LBUTTONDBLCLK:		// 左ダブルクリック
			WmLButtonDblclkProc(hwnd,lParam);
			break;

		case WM_ERASEBKGND:	// 背景消去
			return TRUE;	// 背景は消さない

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
