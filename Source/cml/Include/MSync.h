#ifndef _MSYNC_H
#define _MSYNC_H

//#pragma once
#include <windows.h>
#include <cstring> // For strcpy

class MCriticalSection {
	CRITICAL_SECTION	m_cs;

public:
	MCriticalSection() {
		if (!InitializeCriticalSectionAndSpinCount(&m_cs, 4000)) {
			// Handle initialization failure, possibly throw an exception
		}
	}
	~MCriticalSection() {
		DeleteCriticalSection(&m_cs);
	}
	void Lock() { EnterCriticalSection(&m_cs); }
	void Unlock() { LeaveCriticalSection(&m_cs); }
};

class MSignalEvent {
	HANDLE		m_hEvent;

public:
	MSignalEvent() {
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hEvent == NULL) {
			// Handle error - event creation failed
			// Possibly log or throw an exception
		}
	}
	~MSignalEvent() {
		if (m_hEvent) {
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
	}
	HANDLE GetEvent() const { return m_hEvent; }
	BOOL SetEvent() const { return ::SetEvent(m_hEvent); }
	BOOL ResetEvent() const { return ::ResetEvent(m_hEvent); }
};

class MSingleRunController {
protected:
	char	m_szAppName[_MAX_DIR];
	HANDLE	m_Mutex;
public:
	MSingleRunController(const char* pszAppName) {
		m_Mutex = NULL;
		strcpy(m_szAppName, pszAppName);
	}
	virtual ~MSingleRunController() {
		Destroy();
	}
	bool Create(bool bWaitMode = false) {
		if (bWaitMode == true) {
			int tmTotalWait = 0;
			while (true) {
				m_Mutex = CreateMutex(NULL, TRUE, m_szAppName);
				if (m_Mutex == NULL) {
					// Handle error - mutex creation failed
					return false;
				}
				if (GetLastError() == ERROR_ALREADY_EXISTS) {
					CloseHandle(m_Mutex);

					Sleep(100);
					tmTotalWait += 100;
					if (tmTotalWait >= 5 * 60 * 1000) // Timeout after 5 minutes
						return false;
				}
				else {
					return true;
				}
			}
		}
		else {
			m_Mutex = CreateMutex(NULL, TRUE, m_szAppName);
			if (m_Mutex == NULL) {
				// Handle error - mutex creation failed
				return false;
			}
			if (GetLastError() == ERROR_ALREADY_EXISTS) {
				CloseHandle(m_Mutex);  // Avoid resource leak
				return false;
			}
			return true;
		}
	}
	void Destroy() {
		if (m_Mutex) {
			ReleaseMutex(m_Mutex);
			CloseHandle(m_Mutex);  // Close the handle to avoid resource leak
			m_Mutex = NULL;
		}
	}
};

#endif