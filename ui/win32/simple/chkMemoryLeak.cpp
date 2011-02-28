#ifdef _DEBUG
//#include "stdafx.h"
#include "chkMemoryLeak.h"

#ifdef	__AFXWIN_H__            // MFC�̃E�B���h�E���g���ꍇ�Ɍ��肵�Ă��܂�
#else
 #if defined(_DEBUG)
 #define __chkMemoryLeak_H__
 void* operator new(size_t size, const char *filename, int linenumber)
 {
   return _malloc_dbg(size, _NORMAL_BLOCK, filename, linenumber);
 }
 void   operator delete(void * _P, const char *filename, int linenumber)
 {
   _free_dbg(_P, _NORMAL_BLOCK);
   return;
 }

 #endif
#endif
#endif
