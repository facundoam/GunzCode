/*
 *	CMError.c
 *		Error처리를 위한 함수
 *		이장호 ( 98-01-04 1:19:44 오전 )
 *
 *		SetError(CodeNum)
 *		SetErrors(CodeNum,SubStr)
 *			를 사용해 에러 상태를 저장한다.
 ********************************************************************/

 /*
 #ifdef _USE_MFC
 #include "stdafx.h"
 #endif
 */
#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "CMError.h"

static int g_nErrCode = CM_OK;			/*	Error Code										*/
static char g_pErrStr[256];			/*	Error String									*/
static char g_pErrSubStr[256];			/*	Error Sub String								*/
static char g_pFileName[256];			/*	FileName that Error Occured						*/
static int g_nLineNum;					/*	Line Number that Error Occured					*/
static char g_pLastModification[256];	/*	FileName's Last Modification that Error Occured	*/
static char g_pGetLastErrStr[256];


/*
에러 지정
	nErrCode			에러 코드
	pErrSubStr			에러 코드에 따른 부가 스트링
	pFileName			에러가 일어난 파일 명(__FILE__)
	nLineNum			에러가 일어난 라인 수(__LINE__)
	pLastModification	에러가 일어난 파일의 최근 수정일(__TIMESTAMP__)
	pErrStr				에러 스트링(에러코드 + _KSTR or _ESTR)
*/
void _SetError(int nErrCode, const char* pErrSubStr, const char* pFileName, int nLineNum, const char* pLastModification, const char* pErrStr)
{
	// Set error code
	g_nErrCode = nErrCode;

	// Safely copy the error substrings to avoid buffer overflow
	if (pErrSubStr != NULL) {
		strncpy(g_pErrSubStr, pErrSubStr, sizeof(g_pErrSubStr) - 1);
		g_pErrSubStr[sizeof(g_pErrSubStr) - 1] = '\0';  // Ensure null-termination
	}

	strncpy(g_pFileName, pFileName, sizeof(g_pFileName) - 1);
	g_pFileName[sizeof(g_pFileName) - 1] = '\0';

	g_nLineNum = nLineNum;

	strncpy(g_pLastModification, pLastModification, sizeof(g_pLastModification) - 1);
	g_pLastModification[sizeof(g_pLastModification) - 1] = '\0';

	strncpy(g_pErrStr, pErrStr, sizeof(g_pErrStr) - 1);
	g_pErrStr[sizeof(g_pErrStr) - 1] = '\0';

#ifdef _WIN32
	// Win32 GetLastError handling
	LPVOID lpMsgBuf = nullptr;
	DWORD dwFormatResult = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);

	if (dwFormatResult != 0 && lpMsgBuf != NULL) {
		// Safely copy the error message
		strncpy(g_pGetLastErrStr, (char*)lpMsgBuf, sizeof(g_pGetLastErrStr) - 1);
		g_pGetLastErrStr[sizeof(g_pGetLastErrStr) - 1] = '\0';  // Ensure null-termination
	}
	else {
		// If FormatMessage fails, provide a fallback error message
		strncpy(g_pGetLastErrStr, "Unknown error", sizeof(g_pGetLastErrStr) - 1);
		g_pGetLastErrStr[sizeof(g_pGetLastErrStr) - 1] = '\0';
	}

	// Free the buffer allocated by FormatMessage
	if (lpMsgBuf != nullptr) {
		LocalFree(lpMsgBuf);
	}
#endif
}
void SetErrorSubStr(const char* pErrSubStr)
{
	if (pErrSubStr != NULL)
		strcpy(g_pErrSubStr, pErrSubStr);
}

/*
에러 코드 얻기
*/
int GetErrorCode(void)
{
	return g_nErrCode;
}

/*
에러 스트링 얻기
*/
char* GetErrorString(void)
{
	return g_pErrStr;
}

/*
부가 에러 스트링 얻기
*/
char* GetErrorSubString(void)
{
	return g_pErrSubStr;
}

/*
에러가 난 파일명 얻기
*/
char* GetFileName(void)
{
	return g_pFileName;
}

/*
에러가 난 라인 수 얻기
*/
int GetLineNumber(void)
{
	return g_nLineNum;
}

/*
에러가 난 파일의 최근 수정 일 얻기
*/
char* GetLastModification(void)
{
	return g_pLastModification;
}

#ifdef _WIN32
/*
에러 메세지 출력
*/
void ErrMsgBox(HWND hWnd)
{
	char strTemp[1024] = { 0, };

	if (g_nErrCode == CM_OK) return;

	sprintf(strTemp, "[ %d ]  %s\n", g_nErrCode, g_pErrStr);
	if (g_pErrSubStr[0] != 0)
		sprintf(strTemp, "%s\nSecond Error Message	: %s\n", strTemp, g_pErrSubStr);
	// Win32 GetLastErr...
	sprintf(strTemp, "%s\nGet Last Error		: %s", strTemp, g_pGetLastErrStr);
#ifdef _DEBUG
	sprintf(strTemp, "%s\n\nError Occured Source	: %s ( %d line )", strTemp, g_pFileName, g_nLineNum);
	sprintf(strTemp, "%s\nLast Modify Date		: %s", strTemp, g_pLastModification);
#endif

	MessageBox(hWnd, strTemp, ERROR_MESSAGE_TITLE, MB_ICONERROR);
}
#endif	/* _WIN32 */
