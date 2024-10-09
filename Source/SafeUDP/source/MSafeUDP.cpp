#include "stdafx.h"
#include "MSafeUDP.h"
#include "MBasePacket.h"
#include <time.h>
#include "MCrashDump.h"
#ifndef _PUBLISH
#include "MProcessController.h"
#endif

#ifdef _DEBUG
#include "assert.h"
#endif


// 에이스사가 컴파일 할 경우에는 이 _OLD_SAFEUDP를 활성화해주세요 - 남기룡(2005/07/26)
//#define _OLD_SAFEUDP


#define MAX_RECVBUF_LEN		65535

#define SAFEUDP_MAX_SENDQUEUE_LENGTH		5120
#define SAFEUDP_MAX_ACKQUEUE_LENGTH			5120
#define SAFEUDP_MAX_ACKWAITQUEUE_LENGTH		64

#define SAFEUDP_SAFE_MANAGE_TIME			100		// WSA_INFINITE for debug
#define SAFEUDP_SAFE_RETRANS_TIME			500
#define SAFEUDP_MAX_SAFE_RETRANS_TIME		5000


#define LINKSTATE_LOG
static void MTRACE(char* pszLog)
{
#ifdef _DEBUG
	char szBuf[_MAX_DIR];
	_snprintf_s(szBuf, _MAX_DIR, _TRUNCATE, "%s", pszLog);
	OutputDebugString(szBuf);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////
// MNetLink ////////////////////////////////////////////////////////////////////////////////
MNetLink::MNetLink()
{
	m_pSafeUDP = NULL;
	m_bConnected = false;
	m_nLinkState = LINKSTATE_CLOSED;
	memset((char*)&m_Address, 0, sizeof(sockaddr_in));
	m_nNextReadIndex = 0;
	m_nNextWriteIndex = 0;
	m_dwAuthKey = 0;
	m_pUserData = NULL;

	MTime::GetTime(&m_tvConnectedTime);
	MTime::GetTime(&m_tvLastPacketRecvTime);
}

MNetLink::~MNetLink()
{
	for (ACKWaitListItor itor = m_ACKWaitQueue.begin(); itor != m_ACKWaitQueue.end(); ) {
		delete (*itor);
		itor = m_ACKWaitQueue.erase(itor);
	}
}

void MNetLink::SetLinkState(MNetLink::LINKSTATE nState)
{
	if (m_nLinkState == nState)
		return;

	m_nLinkState = nState;

#ifdef LINKSTATE_LOG
	switch (m_nLinkState) {
	case LINKSTATE_CLOSED:
		Setconnected(false);
		MTRACE("<LINK_STATE> LINKSTATE_CLOSED \n");
		if (m_pSafeUDP && m_pSafeUDP->m_fnNetLinkStateCallback)
			m_pSafeUDP->m_fnNetLinkStateCallback(this, LINKSTATE_CLOSED);
		break;
	case LINKSTATE_ESTABLISHED:
		Setconnected(true);
		MTRACE("<LINK_STATE> LINKSTATE_ESTABLISHED \n");
		if (m_pSafeUDP && m_pSafeUDP->m_fnNetLinkStateCallback)
			m_pSafeUDP->m_fnNetLinkStateCallback(this, LINKSTATE_ESTABLISHED);
		break;
	case LINKSTATE_SYN_SENT:
		MTRACE("<LINK_STATE> LINKSTATE_SYN_SENT \n");
		break;
	case LINKSTATE_SYN_RCVD:
		MTRACE("<LINK_STATE> LINKSTATE_SYN_RCVD \n");
		break;
	case LINKSTATE_FIN_SENT:
		MTRACE("<LINK_STATE> LINKSTATE_FIN_SENT \n");
		break;
	case LINKSTATE_FIN_RCVD:
		MTRACE("<LINK_STATE> LINKSTATE_FIN_RCVD \n");
		break;
	};
#endif
}

bool MNetLink::MakeSockAddr(char* pszIP, int nPort, sockaddr_in* pSockAddr)
{
	sockaddr_in RemoteAddr;
	memset((char*)&RemoteAddr, 0, sizeof(sockaddr_in));

	// Set IP address family and port
	RemoteAddr.sin_family = AF_INET;
	RemoteAddr.sin_port = htons(nPort);

	// Try to convert the IP address using inet_pton
	if (inet_pton(AF_INET, pszIP, &RemoteAddr.sin_addr) == 1) {
		// IP address was successfully parsed
	}
	else {
		// IP parsing failed, attempt to resolve as a hostname
		struct addrinfo hints, * result = NULL;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET; // IPv4

		int res = getaddrinfo(pszIP, NULL, &hints, &result);
		if (res != 0 || result == NULL) {
#ifdef _DEBUG
			OutputDebugString("<SAFEUDP_ERROR> Can't resolve hostname </SAFEUDP_ERROR>\n");
#endif
			return false;
		}

		// Copy resolved IP address to sockaddr_in structure
		memcpy(&RemoteAddr.sin_addr, &((struct sockaddr_in*)result->ai_addr)->sin_addr, sizeof(RemoteAddr.sin_addr));
		freeaddrinfo(result);  // Free the address info
	}

	// Copy the filled sockaddr_in structure to the caller's sockaddr_in
	memcpy(pSockAddr, &RemoteAddr, sizeof(sockaddr_in));
	return true;
}

bool MNetLink::SetAddress(char* pszIP, int nPort)
{
	return MakeSockAddr(pszIP, nPort, &m_Address);
}

__int64 MNetLink::GetMapKey()
{
	__int64 nKey = GetRawPort();
	nKey = nKey << 32;
	nKey += GetIP();
	return nKey;
}

__int64 MNetLink::GetMapKey(sockaddr_in* pSockAddr)
{
	__int64 nKey = pSockAddr->sin_port;
	nKey = nKey << 32;
	nKey += pSockAddr->sin_addr.S_un.S_addr;
	return nKey;
}

bool MNetLink::SendControl(MControlPacket::CONTROL nControl)
{
	MControlPacket* pPacket = new MControlPacket;

	if (nControl == MControlPacket::CONTROL_SYN) {	// Connect.1
		pPacket->nControl = nControl;
		SetLinkState(LINKSTATE_SYN_SENT);
	}
	else if (nControl == MControlPacket::CONTROL_FIN) {	// Disconnect.1
		pPacket->nControl = nControl;
		SetLinkState(LINKSTATE_FIN_SENT);
	}

	return m_pSafeUDP->Send(this, pPacket, sizeof(MControlPacket));
}

bool MNetLink::OnRecvControl(MControlPacket* pPacket)
{
	MControlPacket::CONTROL nControl = pPacket->nControl;
	MControlPacket* pReply = new MControlPacket;
	bool bCheckState = false;

	if (nControl == MControlPacket::CONTROL_SYN) {
		SetLinkState(LINKSTATE_SYN_RCVD);
		pReply->nControl = MControlPacket::CONTROL_SYN_RCVD;
		bCheckState = true;
	}
	else if (nControl == MControlPacket::CONTROL_FIN) {
		SetLinkState(LINKSTATE_FIN_RCVD);
		pReply->nControl = MControlPacket::CONTROL_FIN_RCVD;
		bCheckState = true;
	}
	else {
		switch (m_nLinkState) {
		case LINKSTATE_SYN_SENT:
		{
			if (nControl == MControlPacket::CONTROL_SYN_RCVD) {
				SetLinkState(LINKSTATE_ESTABLISHED);
				pReply->nControl = MControlPacket::CONTROL_ACK;
				bCheckState = true;
			}
		}
		break;
		case LINKSTATE_SYN_RCVD:
		{
			if (nControl == MControlPacket::CONTROL_ACK) {
				SetLinkState(LINKSTATE_ESTABLISHED);
				bCheckState = false;
			}
		}
		break;
		case LINKSTATE_FIN_SENT:
		{
			if (nControl == MControlPacket::CONTROL_FIN_RCVD) {
				SetLinkState(LINKSTATE_CLOSED);
				pReply->nControl = MControlPacket::CONTROL_ACK;
				bCheckState = true;
			}
		}
		break;
		case LINKSTATE_FIN_RCVD:
		{
			if (nControl == MControlPacket::CONTROL_ACK) {
				SetLinkState(LINKSTATE_CLOSED);
				bCheckState = false;
			}
		}
		break;
		};
	}
	if (bCheckState == false) {
		delete pReply;
		return false;
	}

	return m_pSafeUDP->Send(this, pReply, sizeof(MControlPacket));
}

bool MNetLink::SetACKWait(MSafePacket* pPacket, DWORD dwPacketSize)
{
	if (m_ACKWaitQueue.size() > SAFEUDP_MAX_ACKWAITQUEUE_LENGTH)
		return false;

	pPacket->nSafeIndex = GetNextWriteIndex();
	MACKWaitItem* pACKWaitItem = new MACKWaitItem;
	pACKWaitItem->pPacket = pPacket;
	pACKWaitItem->dwPacketSize = dwPacketSize;
	pACKWaitItem->nSendCount = 1;		// SendQueue 에 넣었지만 보냈다고 가정
	MTime::GetTime(&pACKWaitItem->tvFirstSent);
	MTime::GetTime(&pACKWaitItem->tvLastSent);
	m_ACKWaitQueue.push_back(pACKWaitItem);

	return true;
}

bool MNetLink::ClearACKWait(BYTE nSafeIndex)
{
	for (ACKWaitListItor itor = m_ACKWaitQueue.begin(); itor != m_ACKWaitQueue.end(); ) {
		MACKWaitItem* pACKWaitItem = *itor;
		if (pACKWaitItem->pPacket->nSafeIndex == nSafeIndex) {
			delete pACKWaitItem;	// pACKWaitItem->pPacket will Delete too
			itor = m_ACKWaitQueue.erase(itor);
			return true;
		}
		else {
			++itor;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// MSocketThread class /////////////////////////////////////////////////////////////////////
void MSocketThread::Create()
{
	InitializeCriticalSection(&m_csACKLock);
	InitializeCriticalSection(&m_csSendLock);

	InitializeCriticalSection(&m_csCrashDump);
	MThread::Create();
}

void MSocketThread::Destroy()
{
	m_KillEvent.SetEvent();
	MThread::Destroy();		// Wait for Thread Death

	DeleteCriticalSection(&m_csCrashDump);

	DeleteCriticalSection(&m_csSendLock);
	DeleteCriticalSection(&m_csACKLock);
}

void MSocketThread::Run()
{
	try
	{
		while (true) {  // Waiting for SafeUDP Setting...
			DWORD dwVal = WaitForSingleObject(m_KillEvent.GetEvent(), 100);
			if (dwVal == WAIT_OBJECT_0) {
				return;
			}
			else if (dwVal == WAIT_TIMEOUT) {
				if (m_pSafeUDP)
					break;
			}
		}

		WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
		WORD wEventIndex = 0;

		bool bSendable = false;
		WSANETWORKEVENTS NetEvent;
		WSAEVENT hFDEvent = WSACreateEvent();
		if (hFDEvent == WSA_INVALID_EVENT) {
			// Log the error, handle event creation failure
			mlog("Failed to create event\n");
			return;
		}

		EventArray[wEventIndex++] = hFDEvent;
		EventArray[wEventIndex++] = m_ACKEvent.GetEvent();
		EventArray[wEventIndex++] = m_SendEvent.GetEvent();
		EventArray[wEventIndex++] = m_KillEvent.GetEvent();

		if (WSAEventSelect(m_pSafeUDP->GetLocalSocket(), hFDEvent, FD_READ | FD_WRITE) == SOCKET_ERROR) {
			mlog("WSAEventSelect failed with error\n");
			WSACloseEvent(hFDEvent);
			return;
		}

		while (true) {
			DWORD dwReturn = WSAWaitForMultipleEvents(wEventIndex, EventArray, FALSE, SAFEUDP_SAFE_MANAGE_TIME, FALSE);
			if (dwReturn == WSA_WAIT_TIMEOUT) {
				m_pSafeUDP->LockNetLink();
				SafeSendManage();
				m_pSafeUDP->UnlockNetLink();
			}
			else if (dwReturn == WSA_WAIT_EVENT_0) {
				WSAEnumNetworkEvents(m_pSafeUDP->GetLocalSocket(), hFDEvent, &NetEvent);
				if ((NetEvent.lNetworkEvents & FD_READ) == FD_READ) {
					m_pSafeUDP->LockNetLink();
					Recv();
					m_pSafeUDP->UnlockNetLink();
				}
				if ((NetEvent.lNetworkEvents & FD_WRITE) == FD_WRITE) {
					bSendable = true;
				}
			}
			else if (dwReturn == WSA_WAIT_EVENT_0 + 1) {
				FlushACK();
			}
			else if (dwReturn == WSA_WAIT_EVENT_0 + 2) {
				if (bSendable == true)
					FlushSend();
			}
			else if (dwReturn == WSA_WAIT_EVENT_0 + 3) {
				break;
			}
		}

		// Cleanup resources after use
		WSACloseEvent(hFDEvent);

		// Clear Queues
		LockSend();
		{
			for (SendListItor itor = m_SendList.begin(); itor != m_SendList.end();) {
				delete (*itor);
				itor = m_SendList.erase(itor);
			}
		}
		{
			for (SendListItor itor = m_TempSendList.begin(); itor != m_TempSendList.end();) {
				delete (*itor);
				itor = m_TempSendList.erase(itor);
			}
		}
		UnlockSend();

		LockACK();
		{
			for (ACKSendListItor itor = m_ACKSendList.begin(); itor != m_ACKSendList.end();) {
				delete (*itor);
				itor = m_ACKSendList.erase(itor);
			}
		}
		{
			for (ACKSendListItor itor = m_TempACKSendList.begin(); itor != m_TempACKSendList.end();) {
				delete (*itor);
				itor = m_TempACKSendList.erase(itor);
			}
		}
		UnlockACK();
	}
	catch (const std::exception& e)
	{
		// Log the exception with details
		mlog("Exception caught in MSocketThread::Run: %s\n", e.what());

#ifndef _PUBLISH
		char szFileName[_MAX_DIR];
		GetModuleFileName(NULL, szFileName, _MAX_DIR);
		HANDLE hProcess = MProcessController::OpenProcessHandleByFilePath(szFileName);
		TerminateProcess(hProcess, 0);
#endif
	}
	catch (...)
	{
		// Log the unknown exception
		mlog("Unknown exception caught in MSocketThread::Run\n");

#ifndef _PUBLISH
		char szFileName[_MAX_DIR];
		GetModuleFileName(NULL, szFileName, _MAX_DIR);
		HANDLE hProcess = MProcessController::OpenProcessHandleByFilePath(szFileName);
		TerminateProcess(hProcess, 0);
#endif
	}
}
DWORD MSocketThread::CrashDump(PEXCEPTION_POINTERS ExceptionInfo)
{
	mlog("CrashDump Entered 1\n");
	EnterCriticalSection(&m_csCrashDump);
	mlog("CrashDump Entered 2\n");

	if (PathIsDirectory("Log") == FALSE)
		CreateDirectory("Log", NULL);

	time_t		tClock;
	struct tm* ptmTime;

	time(&tClock);
	ptmTime = localtime(&tClock);

	char szFileName[_MAX_DIR];

	int nFooter = 1;
	while (TRUE) {
		sprintf(szFileName, "Log/MSocketThread_%02d-%02d-%02d-%d.dmp",
			ptmTime->tm_year + 1900, ptmTime->tm_mon + 1, ptmTime->tm_mday, nFooter);

		if (PathFileExists(szFileName) == FALSE)
			break;

		nFooter++;
		if (nFooter > 100)
		{
			LeaveCriticalSection(&m_csCrashDump);
			return false;
		}
	}

	DWORD ret = CrashExceptionDump(ExceptionInfo, szFileName, true);

	mlog("CrashDump Leaving\n");
	LeaveCriticalSection(&m_csCrashDump);
	mlog("CrashDump Leaved\n");

	return ret;
}

void MSocketThread::Debug()
{

}

bool MSocketThread::PushACK(MNetLink* pNetLink, MSafePacket* pPacket)
{
	if (m_ACKSendList.size() > SAFEUDP_MAX_ACKQUEUE_LENGTH)
		return false;

	MACKQueueItem* pACKItem = new MACKQueueItem;
	pACKItem->dwIP = pNetLink->GetIP();
	pACKItem->wRawPort = pNetLink->GetRawPort();
	pACKItem->nSafeIndex = pPacket->nSafeIndex;

	LockACK();
	m_TempACKSendList.push_back(pACKItem);
	UnlockACK();

	m_ACKEvent.SetEvent();

	return true;
}

bool MSocketThread::PushSend(MNetLink* pNetLink, MBasePacket* pPacket, DWORD dwPacketSize, bool bRetransmit)
{
	if (!pNetLink || (m_SendList.size() > SAFEUDP_MAX_SENDQUEUE_LENGTH))
		return false;

	MSendQueueItem* pSendItem = new MSendQueueItem;
	pSendItem->dwIP = pNetLink->GetIP();
	pSendItem->wRawPort = pNetLink->GetRawPort();
	pSendItem->pPacket = pPacket;
	pSendItem->dwPacketSize = dwPacketSize;

	if ((pPacket->GetFlag(SAFEUDP_FLAG_SAFE_PACKET) != FALSE) && (bRetransmit == false)) {
		pNetLink->SetACKWait((MSafePacket*)pPacket, dwPacketSize);
	}

	LockSend();
	m_TempSendList.push_back(pSendItem);
	UnlockSend();

	m_SendEvent.SetEvent();

	return true;
}

bool MSocketThread::PushSend(char* pszIP, int nPort, char* pPacket, DWORD dwPacketSize)
{
	if (m_SendList.size() > SAFEUDP_MAX_SENDQUEUE_LENGTH)
		return false;

	sockaddr_in Addr;
	if (MNetLink::MakeSockAddr(pszIP, nPort, &Addr) == false)
		return false;

	MSendQueueItem* pSendItem = new MSendQueueItem;
	pSendItem->dwIP = Addr.sin_addr.S_un.S_addr;
	pSendItem->wRawPort = Addr.sin_port;
	pSendItem->pPacket = (MBasePacket*)pPacket;
	pSendItem->dwPacketSize = dwPacketSize;

	LockSend();
	m_TempSendList.push_back(pSendItem);
	UnlockSend();

	m_SendEvent.SetEvent();

	return true;
}


bool MSocketThread::PushSend(DWORD dwIP, int nPort, char* pPacket, DWORD dwPacketSize)
{
	if ((SAFEUDP_MAX_SENDQUEUE_LENGTH < m_SendList.size()) ||
		(INADDR_NONE == dwIP))
		return false;

	sockaddr_in Addr;
	memset((char*)&Addr, 0, sizeof(sockaddr_in));

	//	Set Dest IP and Port 
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(nPort);
	memcpy(&(Addr.sin_addr), &dwIP, 4);

	MSendQueueItem* pSendItem = new MSendQueueItem;
	if (0 != pSendItem)
	{
		pSendItem->dwIP = dwIP;
		pSendItem->wRawPort = Addr.sin_port;
		pSendItem->pPacket = (MBasePacket*)pPacket;
		pSendItem->dwPacketSize = dwPacketSize;

		LockSend();
		m_TempSendList.push_back(pSendItem);
		UnlockSend();

		m_SendEvent.SetEvent();

		return true;
	}

	return false;
}


bool MSocketThread::FlushACK()
{
	LockACK();
	while (m_TempACKSendList.size() > 0) {
		ACKSendListItor itor = m_TempACKSendList.begin();
		m_ACKSendList.push_back(*itor);
		m_TempACKSendList.erase(itor);
	}
	UnlockACK();

	while (m_ACKSendList.size() > 0) {
		ACKSendListItor itor = m_ACKSendList.begin();
		MACKQueueItem* pACKItem = *itor;

		MACKPacket ACKPacket;
		ACKPacket.nSafeIndex = pACKItem->nSafeIndex;

		sockaddr_in DestAddr;
		DestAddr.sin_family = AF_INET;
		DestAddr.sin_addr.S_un.S_addr = pACKItem->dwIP;
		DestAddr.sin_port = pACKItem->wRawPort;

		int nResult = sendto(m_pSafeUDP->GetLocalSocket(), (char*)&ACKPacket, sizeof(MACKPacket),
			0, (sockaddr*)&DestAddr, sizeof(sockaddr_in));
		if (nResult == SOCKET_ERROR) {
			return false;
		}
		else {
			m_nTotalSend += nResult;
			m_SendTrafficLog.Record(m_nTotalSend);
		}

		delete pACKItem; //delete *itor;
		m_ACKSendList.erase(itor);
	}

	return true;
}

bool MSocketThread::FlushSend()
{
	LockSend();
	while (m_TempSendList.size() > 0) {
		SendListItor itor = m_TempSendList.begin();
		m_SendList.push_back(*itor);
		m_TempSendList.erase(itor);
	}
	UnlockSend();

	MSendQueueItem* pSendItem;
	sockaddr_in		DestAddr;
	int				nResult;
	while (m_SendList.size() > 0) {
		SendListItor itor = m_SendList.begin();
		pSendItem = *itor;

		DestAddr.sin_family = AF_INET;
		DestAddr.sin_addr.S_un.S_addr = pSendItem->dwIP;
		DestAddr.sin_port = pSendItem->wRawPort;

		nResult = sendto(m_pSafeUDP->GetLocalSocket(), (char*)pSendItem->pPacket, pSendItem->dwPacketSize,
			0, (sockaddr*)&DestAddr, sizeof(sockaddr_in));
		if (nResult == SOCKET_ERROR) {
			int nErrCode = WSAGetLastError();
			char szBuf[64]; sprintf(szBuf, "<SAFEUDP_ERROR>FlushSend() - sendto() ErrCode=%d \n", nErrCode);
			MTRACE(szBuf);
			//			return false;
		}
		else {
			m_nTotalSend += nResult;
			m_SendTrafficLog.Record(m_nTotalSend);
		}

#ifdef _OLD_SAFEUDP
		if (pSendItem->pPacket->GetFlag(SAFEUDP_FLAG_SAFE_PACKET) != FALSE) {
			// Don't Delete SafePacket (pSendItem->pPacket)
			delete pSendItem;
			m_SendList.erase(itor);
		}
		else {	// Means Normal Packet
			delete pSendItem->pPacket;
			delete pSendItem; //delete (*itor);
			m_SendList.erase(itor);
		}
#else
		// 건즈에서는 SAFE_UDP_FLAG_SAFE_PACKET를 사용하지 않는다.
		delete pSendItem->pPacket;
		delete pSendItem;
		m_SendList.erase(itor);
#endif

	}
	return true;
}

bool MSocketThread::SafeSendManage()
{
	int nCnt = m_pSafeUDP->m_NetLinkMap.size();
	for (NetLinkItor itorLink = m_pSafeUDP->m_NetLinkMap.begin(); itorLink != m_pSafeUDP->m_NetLinkMap.end(); ) {
		MNetLink* pNetLink = (*itorLink).second;

		timeval tvNow;
		MTime::GetTime(&tvNow);

		// Closed Idle time check
		timeval tvIdleDiff;
		tvIdleDiff = MTime::TimeSub(tvNow, pNetLink->m_tvLastPacketRecvTime);
		if ((pNetLink->GetLinkState() != MNetLink::LINKSTATE_ESTABLISHED) &&
			((tvIdleDiff.tv_sec * 1000 + tvIdleDiff.tv_usec) > SAFEUDP_MAX_SAFE_RETRANS_TIME)) {
			MTRACE("SUDP> Idle Control Timeout \n");
			pNetLink->SetLinkState(MNetLink::LINKSTATE_CLOSED);

			delete (*itorLink).second;
			m_pSafeUDP->m_NetLinkMap.erase(itorLink++);
			continue;
		}
		else {
			++itorLink;
		}

		for (MNetLink::ACKWaitListItor itorACK = pNetLink->m_ACKWaitQueue.begin(); itorACK != pNetLink->m_ACKWaitQueue.end(); ++itorACK) {
			MACKWaitItem* pACKWaitItem = *itorACK;

			timeval tvDiff;
			tvDiff = MTime::TimeSub(tvNow, pACKWaitItem->tvFirstSent);
			if ((tvDiff.tv_sec * 1000 + tvDiff.tv_usec) > SAFEUDP_MAX_SAFE_RETRANS_TIME) {
				// Disconnect....
				MTRACE("SUDP> Retransmit Timeout \n");
				pNetLink->SetLinkState(MNetLink::LINKSTATE_CLOSED);
				break;
			}

			tvDiff = MTime::TimeSub(tvNow, pACKWaitItem->tvLastSent);
			if ((tvDiff.tv_sec * 1000 + tvDiff.tv_usec) > SAFEUDP_SAFE_RETRANS_TIME) {
				PushSend(pNetLink, pACKWaitItem->pPacket, pACKWaitItem->dwPacketSize, true);
				pACKWaitItem->tvLastSent = tvNow;
				pACKWaitItem->nSendCount++;
			}
		}
	}
	return true;
}

bool MSocketThread::Recv()
{
	sockaddr_in		AddrFrom;
	int				nAddrFromLen = sizeof(sockaddr);

	char			RecvBuf[MAX_RECVBUF_LEN];
	int				nRecv = 0;

	while (TRUE) {
		nRecv = recvfrom(m_pSafeUDP->GetLocalSocket(), RecvBuf, MAX_RECVBUF_LEN, 0,
			(struct sockaddr*)&AddrFrom, &nAddrFromLen);
		if (nRecv != SOCKET_ERROR) {
			m_nTotalRecv += nRecv;
			m_RecvTrafficLog.Record(m_nTotalRecv);
		}

		if (nRecv <= 0) break;					// Stop Receive Loop

		if (m_fnCustomRecvCallback && OnCustomRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, RecvBuf, nRecv) == true) {
			continue;
		}
		else if (((MBasePacket*)RecvBuf)->GetFlag(SAFEUDP_FLAG_CONTROL_PACKET) != FALSE) {
			OnControlRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, (MBasePacket*)RecvBuf, nRecv);
			continue;
		}
		else if (((MBasePacket*)RecvBuf)->GetFlag(SAFEUDP_FLAG_LIGHT_PACKET) != FALSE) {
			OnLightRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, (MLightPacket*)RecvBuf, nRecv);
			continue;
		}
		else if (((MBasePacket*)RecvBuf)->GetFlag(SAFEUDP_FLAG_ACK_PACKET) != FALSE) {
			OnACKRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, (MACKPacket*)RecvBuf);
			continue;
		}
		else {
			OnGenericRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, (MBasePacket*)RecvBuf, nRecv);
			continue;
		}
	}

	return true;
}

bool MSocketThread::OnCustomRecv(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize)
{
	if (m_fnCustomRecvCallback)
		return m_fnCustomRecvCallback(dwIP, wRawPort, pPacket, dwSize);
	return false;
}

bool MSocketThread::OnControlRecv(DWORD dwIP, WORD wRawPort, MBasePacket* pPacket, DWORD dwSize)
{
	MControlPacket* pControlPacket = (MControlPacket*)pPacket;
	MNetLink* pNetLink = m_pSafeUDP->FindNetLink(dwIP, wRawPort);

	if (pNetLink == NULL) {
		if (pControlPacket->nControl == MControlPacket::CONTROL_SYN) {
			IN_ADDR addr;
			addr.S_un.S_addr = dwIP;
			pNetLink = m_pSafeUDP->OpenNetLink(inet_ntoa(addr), ntohs(wRawPort));
			pNetLink->OnRecvControl(pControlPacket);
		}
	}
	else {
		pNetLink->OnRecvControl(pControlPacket);
	}

	if (pNetLink && (pPacket->GetFlag(SAFEUDP_FLAG_SAFE_PACKET) != FALSE))
		PushACK(pNetLink, (MSafePacket*)pPacket);

	return true;
}

bool MSocketThread::OnLightRecv(DWORD dwIP, WORD wRawPort, MLightPacket* pPacket, DWORD dwSize)
{
	if (m_fnLightRecvCallback)
		m_fnLightRecvCallback(dwIP, wRawPort, pPacket, dwSize);
	return true;
}

bool MSocketThread::OnACKRecv(DWORD dwIP, WORD wRawPort, MACKPacket* pPacket)
{
	MNetLink* pNetLink = m_pSafeUDP->FindNetLink(dwIP, wRawPort);
	if (pNetLink == NULL)
		return false;

	pNetLink->ClearACKWait(pPacket->nSafeIndex);

	return false;
}

bool MSocketThread::OnGenericRecv(DWORD dwIP, WORD wRawPort, MBasePacket* pPacket, DWORD dwSize)
{
	MNetLink* pNetLink = m_pSafeUDP->FindNetLink(dwIP, wRawPort);
	if (pNetLink == NULL)
		return false;

	if (pPacket->GetFlag(SAFEUDP_FLAG_SAFE_PACKET) != FALSE)
		PushACK(pNetLink, (MSafePacket*)pPacket);

	// recv
	if (m_fnGenericRecvCallback)
		m_fnGenericRecvCallback(pNetLink, pPacket, dwSize);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
// MSafeUDP class //////////////////////////////////////////////////////////////////////////
bool MSafeUDP::Create(bool bBindWinsockDLL, int nPort, bool bReusePort)
{
	m_bBindWinsockDLL = bBindWinsockDLL;

	WSADATA	wsaData;
	if ((bBindWinsockDLL == true) && (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0))
		return false;

	if (OpenSocket(nPort, bReusePort) == false) {
#ifdef _DEBUG
		OutputDebugString("<SAFEUDP_ERROR> OpenSocket() FAILED </SAFEUDP_ERROR>\n");
#endif
		return false;
	}

	InitializeCriticalSection(&m_csNetLink);

	m_SocketThread.SetSafeUDP(this);
	m_SocketThread.Create();
	return true;
}

void MSafeUDP::Destroy()
{
	if (m_SocketThread.GetSafeUDP() == NULL)
		return;

	DisconnectAll();

	m_SocketThread.Destroy();
	CloseSocket();

	DeleteCriticalSection(&m_csNetLink);

	if (m_bBindWinsockDLL == true) {
		WSACleanup();
		m_bBindWinsockDLL = false;
	}
}

bool MSafeUDP::OpenSocket(int nPort, bool bReuse)
{
	SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == INVALID_SOCKET)
		return false;

	if (bReuse) {
		int opt = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR)
			return false;

		int rcv_size = 64 * 1024;
		if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&rcv_size, sizeof(rcv_size)) == SOCKET_ERROR)
			return false;
	}

	sockaddr_in LocalAddress;
	LocalAddress.sin_family = AF_INET;
	LocalAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	LocalAddress.sin_port = htons(nPort);

	if (bind(sockfd, (struct sockaddr*)&LocalAddress, sizeof(LocalAddress)) == SOCKET_ERROR) {
		closesocket(sockfd);
		return false;
	}

	m_Socket = sockfd;
	m_LocalAddress = LocalAddress;

	return true;
}

void MSafeUDP::CloseSocket()
{
	shutdown(m_Socket, SD_SEND);
	closesocket(m_Socket);
	m_Socket = 0;
}

bool MSafeUDP::Send(MNetLink* pNetLink, MBasePacket* pPacket, DWORD dwPacketSize)
{
	return m_SocketThread.PushSend(pNetLink, pPacket, dwPacketSize, false);
}

bool MSafeUDP::Send(char* pszIP, int nPort, char* pPacket, DWORD dwSize)
{
	return m_SocketThread.PushSend(pszIP, nPort, pPacket, dwSize);
}

bool MSafeUDP::Send(DWORD dwIP, int nPort, char* pPacket, DWORD dwSize)
{
	return m_SocketThread.PushSend(dwIP, nPort, pPacket, dwSize);
}

MNetLink* MSafeUDP::FindNetLink(DWORD dwIP, WORD wRawPort)
{
	__int64 nKey = wRawPort;
	nKey = nKey << 32;
	nKey += dwIP;

	NetLinkItor pos = m_NetLinkMap.find(nKey);
	if (pos != m_NetLinkMap.end())
		return (*pos).second;

	return NULL;
}

MNetLink* MSafeUDP::FindNetLink(__int64 nMapKey)
{
	NetLinkItor pos = m_NetLinkMap.find(nMapKey);
	if (pos != m_NetLinkMap.end())
		return (*pos).second;
	return NULL;
}

MNetLink* MSafeUDP::OpenNetLink(char* szIP, int nPort)
{
	MNetLink* pNetLink = new MNetLink;
	pNetLink->SetSafeUDP(this);
	pNetLink->SetAddress(szIP, nPort);

	__int64 nKey = pNetLink->GetMapKey();

	NetLinkItor pos = m_NetLinkMap.find(nKey);
	if (pos != m_NetLinkMap.end()) {
		Reconnect((*pos).second);
		delete pNetLink;
		pNetLink = (*pos).second;
	}
	else {
		m_NetLinkMap.insert(NetLinkType(nKey, pNetLink));
	}

	return pNetLink;
}

bool MSafeUDP::CloseNetLink(MNetLink* pNetLink)
{
	__int64 nKey = pNetLink->GetMapKey();

	NetLinkItor pos = m_NetLinkMap.find(nKey);
	if (pos == m_NetLinkMap.end())
		return false;

	delete (*pos).second;
	m_NetLinkMap.erase(pos);

	return true;
}

MNetLink* MSafeUDP::Connect(char* szIP, int nPort)
{
	MNetLink* pNetLink = OpenNetLink(szIP, nPort);
	pNetLink->SendControl(MControlPacket::CONTROL_SYN);

	return pNetLink;
}

void MSafeUDP::Reconnect(MNetLink* pNetLink)
{
	pNetLink->SendControl(MControlPacket::CONTROL_SYN);
}

bool MSafeUDP::Disconnect(MNetLink* pNetLink)
{
	pNetLink->SendControl(MControlPacket::CONTROL_FIN);

	return true;
}

int MSafeUDP::DisconnectAll()
{
	LockNetLink();
	int nCount = 0;
	for (NetLinkItor itor = m_NetLinkMap.begin(); itor != m_NetLinkMap.end(); ) {
		delete (*itor).second;
		m_NetLinkMap.erase(itor++);
		++nCount;
	}
	UnlockNetLink();
	return nCount;
}

