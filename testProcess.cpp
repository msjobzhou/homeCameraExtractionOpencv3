#include <windows.h>
#include <stdio.h>
#include <tchar.h> 
#include "test.h"

int test_createProcess() {
	STARTUPINFO stStartUpInfo;
	::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
	stStartUpInfo.cb = sizeof(stStartUpInfo);

	PROCESS_INFORMATION stProcessInfo;
	::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

	//TCHAR szPath[] = _T("C:\\cygwin64\\bin\\mintty.exe");
	TCHAR szPath[] = _T("D:\\homeCameraExtractionOpencv3.exe");
	try
	{
		bool bRet = ::CreateProcess(
			szPath,
			NULL,
			NULL,
			NULL,
			false,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&stStartUpInfo,
			&stProcessInfo);

		if (bRet)
		{
			//等待3s后关闭进程
			WaitForSingleObject(stProcessInfo.hProcess, 30000L);
			::CloseHandle(stProcessInfo.hProcess);
			::CloseHandle(stProcessInfo.hThread);
			stProcessInfo.hProcess = NULL;
			stProcessInfo.hThread = NULL;
			stProcessInfo.dwProcessId = 0;
			stProcessInfo.dwThreadId = 0;
		}
		else
		{
			//如果创建进程失败，查看错误码
			DWORD dwErrCode = GetLastError();
			printf_s("ErrCode : %d\n", dwErrCode);

		}
	}
	catch (...)
	{
	}

	return 0;
}