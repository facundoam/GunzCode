/*
	CommonLog.h
	-----------

	Programming by Joongpil Cho
	All copyright (c) 1999, MAIET entertainment
*/
#include <stdio.h>

#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__


// Base Class of General Log System
class CMLog {
public:
	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Print(const char* pFormat) = 0;
};

/* CMNullLog */
class CMNullLog : public CMLog {
public:
	virtual void Init() {}
	virtual void Shutdown() {}
	virtual void Print(const char* pFormat) {}
};

/* CMFileLog */
class CMFileLog : public CMLog {
	char* m_pFileName;
public:
	CMFileLog(char* szFileName = NULL) {
		if (szFileName) {
			m_pFileName = _strdup(szFileName);
		}
		else {
			m_pFileName = _strdup("mlog.txt");
		}
	}

	virtual ~CMFileLog() {
		Shutdown();
	}

	virtual void Init() {
		DeleteFile(m_pFileName);
	}

	virtual void Shutdown() {
		if (m_pFileName) {
			free(m_pFileName);
			m_pFileName = NULL;
		}
	}

	virtual void Print(const char* string) {
		FILE* pFile = nullptr;

		// Open file for appending, with more secure fopen_s
		if (fopen_s(&pFile, m_pFileName, "a") != 0 || !pFile) {
			// If opening for append fails, attempt to create a new file
			if (fopen_s(&pFile, m_pFileName, "w") != 0 || !pFile) {
				// Failed to open file for writing, handle the error (logging, etc.)
				return;
			}
			fclose(pFile); // Close the newly created file
			// Try again to open in append mode
			if (fopen_s(&pFile, m_pFileName, "a") != 0 || !pFile) {
				// If it still fails, return
				return;
			}
		}

		// Write to the file using fprintf safely, ensuring proper formatting
		fprintf(pFile, "%s", string);

		// Close the file
		fclose(pFile);
	}
};

class CMDebugStringLog : public CMLog {
public:
	virtual void Init() {
		//DO NOTHING
	}
	virtual void Shutdown() {
		//DO NOTHING
	}
	virtual void Print(const char* pFormat) {
#ifdef _DEBUG
		OutputDebugString(pFormat);
#endif
	}
};



void SetLogHandler(CMLog* pLog);
void Log(const char* pFormat, ...);



#endif // __COMMON_HEADER__