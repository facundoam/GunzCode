/*
	CommonLog.cpp
	-------------

	Programming by Joongpil Cho
	All copyright (c) 1999, MAIET entertainment
*/
#include "stdafx.h"
#include <windows.h>
#include "CommonLog.h"

///////////////////////////////////////////////////////////////////////
// CMLog

static CMLog* pCurrentLog = NULL;

void SetLogHandler(CMLog* pLog)
{
	if (pLog) {
		if (pCurrentLog) pCurrentLog->Shutdown();
		pCurrentLog = pLog;
		pCurrentLog->Init();
	}
}

void Log(const char* pFormat, ...)
{
	va_list args;
	char szTemp[256];

	if (pCurrentLog) {
		va_start(args, pFormat);
		// Use _vsnprintf_s for safer formatting with buffer size
		_vsnprintf_s(szTemp, sizeof(szTemp), _TRUNCATE, pFormat, args);
		va_end(args);

		// Call the Print function with the formatted string
		pCurrentLog->Print(szTemp);
	}
}

