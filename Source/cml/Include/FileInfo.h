#ifndef _FILEINFO_H
#define _FILEINFO_H

#include <windows.h>
#include <time.h>

BOOL GetLastUpdate(const char* pFileName, FILETIME* ft);
BOOL IsModifiedOutside(const char* pFileName, FILETIME ot);
BOOL RemoveExtension(char* pRemoveExt, const char* pFileName);
void ReplaceExtension(char* pTargetName, const char* pSourceName, char* pExt);
void GetRelativePath(char* pRelativePath, const char* pBasePath, const char* pPath);
void GetFullPath(char* pFullPath, const char* pBasePath, const char* pRelativePath);
void GetFullPath(char* pFullPath, const char* pRelativePath);
void GetPurePath(char* pPurePath, const char* pFilename);
void GetPureFilename(char* pPureFilename, const char* pFilename);
void GetPureExtension(char* pPureExtension, const char* pFilename);
BOOL IsFullPath(const char* pPath);
BOOL ReadHeader(HANDLE hFile, void* pHeader, int nHeaderSize);
DWORD GetFileCheckSum(char* pszFileName);
bool IsExist(const char* filename);
void GetParentDirectory(char* pszFileName);
bool MakePath(const char* pszFileName);
void time_tToFILETIME(time_t t, LPFILETIME pft);
BOOL MSetFileTime(LPCTSTR lpszPath, FILETIME ft);
bool GetMyDocumentsPath(char* path);
bool CreatePath(char* path);

#ifdef WIN32
#pragma comment(lib, "Shlwapi.lib")
#endif

#endif