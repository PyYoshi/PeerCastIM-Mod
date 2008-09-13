#include "win32/seh.h"

void SEHdump(_EXCEPTION_POINTERS *lpExcept)
{
	// crash dump
	MINIDUMP_EXCEPTION_INFORMATION minidumpInfo;
	HANDLE hFile;
	BOOL dump = FALSE;

	minidumpInfo.ThreadId = GetCurrentThreadId();
	minidumpInfo.ExceptionPointers = lpExcept;
	minidumpInfo.ClientPointers = FALSE;

	hFile = CreateFile(".\\dump.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		dump = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
			(MINIDUMP_TYPE)
			(MiniDumpWithIndirectlyReferencedMemory
			|MiniDumpWithPrivateReadWriteMemory
			|MiniDumpWithThreadInfo
			|MiniDumpWithUnloadedModules),
			&minidumpInfo, NULL, NULL);
		CloseHandle(hFile);
	}


	// dump peercast's log
	fs.openWriteReplace(".\\dump.html");
	sys->logBuf->dumpHTML(fs);
	fs.close();
	if (dump)
	{
		MessageBox(NULL, "��ʕی�ᔽ�ׁ̈A�v���O�����͋����I������܂��B\n"
			"�������̂��߃_���v�f�[�^(dump.html, dump.dmp)��񋟂��Ă��������B", "SEH",
			MB_OK|MB_ICONWARNING);
	} else
	{
		MessageBox(NULL, "��ʕی�ᔽ�ׁ̈A�v���O�����͋����I������܂��B\n"
			"�������̂��߂Ƀ_���v�f�[�^(dump.html)��񋟂��Ă��������B", "SEH",
			MB_OK|MB_ICONWARNING);
	}

	::exit(lpExcept->ExceptionRecord->ExceptionCode);
}
