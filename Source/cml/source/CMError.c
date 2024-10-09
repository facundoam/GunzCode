/*
 *	CMError.c
 *		Erroró���� ���� �Լ�
 *		����ȣ ( 98-01-04 1:19:44 ���� )
 *
 *		SetError(CodeNum)
 *		SetErrors(CodeNum,SubStr)
 *			�� ����� ���� ���¸� �����Ѵ�.
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
���� ����
	nErrCode			���� �ڵ�
	pErrSubStr			���� �ڵ忡 ���� �ΰ� ��Ʈ��
	pFileName			������ �Ͼ ���� ��(__FILE__)
	nLineNum			������ �Ͼ ���� ��(__LINE__)
	pLastModification	������ �Ͼ ������ �ֱ� ������(__TIMESTAMP__)
	pErrStr				���� ��Ʈ��(�����ڵ� + _KSTR or _ESTR)
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
���� �ڵ� ���
*/
int GetErrorCode(void)
{
	return g_nErrCode;
}

/*
���� ��Ʈ�� ���
*/
char* GetErrorString(void)
{
	return g_pErrStr;
}

/*
�ΰ� ���� ��Ʈ�� ���
*/
char* GetErrorSubString(void)
{
	return g_pErrSubStr;
}

/*
������ �� ���ϸ� ���
*/
char* GetFileName(void)
{
	return g_pFileName;
}

/*
������ �� ���� �� ���
*/
int GetLineNumber(void)
{
	return g_nLineNum;
}

/*
������ �� ������ �ֱ� ���� �� ���
*/
char* GetLastModification(void)
{
	return g_pLastModification;
}

#ifdef _WIN32
/*
���� �޼��� ���
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
