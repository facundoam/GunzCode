#ifndef MMATCHCLIENT_H
#define MMATCHCLIENT_H

#include <list>
#include <map>
using namespace std;
#include "MMatchGlobal.h"
#include "MCommandCommunicator.h"
#include "MClient.h"
#include "MSafeUDP.h"
#include "MMatchObjCache.h"
#include "MMatchObject.h"
#include "MMatchTransDataType.h"
#include "MMatchStage.h"
#include "MMatchGlobal.h"
#include "MPacketCrypter.h"


#define MATCHCLIENT_DEFAULT_UDP_PORT	10000
#define MAX_PING						999

class MMatchPeerInfo
{
public:
	MUID    uidChar;
	char    szIP[64];
	DWORD   dwIP;
	int     nPort;
	MTD_CharInfo   CharInfo;
	//버프정보임시주석 MTD_CharBuffInfo    CharBuffInfo;
	MTD_ExtendInfo ExtendInfo;

protected:
	bool    m_bUDPTestResult;
	bool    m_bUDPTestProcess;
	int     m_nUDPTestCount;

	bool    m_bOpened;
	int     m_nPing;
	int     m_nPingTryCount;
	unsigned int m_nLastPingTime;
	unsigned int m_nLastPongTime;
	MCommandSNChecker m_CommandSNChecker;

public:
	// Constructor that uses an initializer list
	MMatchPeerInfo()
		: uidChar(0, 0),
		dwIP(0),
		nPort(0),
		m_bUDPTestResult(false),
		m_bUDPTestProcess(false),
		m_nUDPTestCount(0),
		m_bOpened(false),
		m_nPing(0),
		m_nPingTryCount(0),
		m_nLastPingTime(0),
		m_nLastPongTime(0)
	{
		memset(&CharInfo, 0, sizeof(MTD_CharInfo));
		//버프정보임시주석 memset(&CharBuffInfo, 0, sizeof(MTD_CharBuffInfo));
		memset(szIP, 0, sizeof(szIP));  // Initialize the szIP array
	}

	virtual ~MMatchPeerInfo() {}

	// Accessor and mutator methods
	bool GetUDPTestResult() const { return m_bUDPTestResult; }
	void SetUDPTestResult(bool bResult) { m_bUDPTestResult = bResult; }
	void StartUDPTest() { m_bUDPTestProcess = true; m_nUDPTestCount = 10; }
	void StopUDPTest() { m_bUDPTestProcess = false; m_nUDPTestCount = 0; }
	bool GetProcess() const { return m_bUDPTestProcess; }
	int GetTestCount() const { return m_nUDPTestCount; }
	void UseTestCount() { m_nUDPTestCount--; }

	bool IsOpened() const { return m_bOpened; }
	void SetOpened(bool bVal) { m_bOpened = bVal; }
	int GetPing(unsigned int nCurrTime);
	void UpdatePing(unsigned int nTime, int nPing);
	void SetLastPingTime(unsigned int nTime);
	bool CheckCommandValidate(MCommand* pCmd)
	{
		return m_CommandSNChecker.CheckValidate(pCmd->m_nSerialNumber);
	}
};


class MMatchPeerInfoList : public map<MUID, MMatchPeerInfo*>
{
private:
	map<MUID, MMatchPeerInfo*>		m_IPnPortMap;
	CRITICAL_SECTION				m_csLock;
	void Lock()			{ EnterCriticalSection(&m_csLock); }
	void Unlock()		{ LeaveCriticalSection(&m_csLock); }
public:
	MMatchPeerInfoList();
	virtual ~MMatchPeerInfoList();
	void Clear();
	void Add(MMatchPeerInfo* pPeerInfo);
	bool Delete(MMatchPeerInfo* pPeerInfo);
	MMatchPeerInfo* Find(const MUID& uidChar);
	MUID FindUID(DWORD dwIP, int nPort);
};

/// 게임 클라이언트
class MMatchClient : public MClient
{
protected:
// 핵 제작을 방해하기 위해 멤버변수의 위치를 빌드때마다 뒤섞기 위한 주석매크로(runtime/ShuffleCode.bat 실행)
// m_uidPlayer와 기타 uid들은 위조 커맨드를 생성하기 위해 알아내야 하는 값이다.
/* [[SHUFFLE_LINE]] MMatchClient */	MUID				m_uidPlayer;
/* [[SHUFFLE_LINE]] MMatchClient */	MMatchServerMode	m_nServerMode;				///< 서버모드
/* [[SHUFFLE_LINE]] MMatchClient */	bool				m_bEnabledSurvivalMode;		///< 서바이벌 모드 활성화 여부 (퀘스트서버에서만 적용)
/* [[SHUFFLE_LINE]] MMatchClient */	char				m_szAgentIP[32];

/* [[SHUFFLE_LINE]] MMatchClient */	int					m_nServerPeerPort;
/* [[SHUFFLE_LINE]] MMatchClient */	bool				m_bAgentPeerFlag;	// Agent와 UDP연결여부
/* [[SHUFFLE_LINE]] MMatchClient */	MUID				m_uidChannel;
/* [[SHUFFLE_LINE]] MMatchClient */	bool				m_bAllowTunneling;
/* [[SHUFFLE_LINE]] MMatchClient */	bool				m_bEnabledDuelTournament;	///< 듀얼토너먼트 활성화 여부

/* [[SHUFFLE_LINE]] MMatchClient */	int					m_nAgentPeerCount;	// Agent와 UDP연결시 Retry를 위한 정보
/* [[SHUFFLE_LINE]] MMatchClient */	int					m_nAgentPeerPort;

/* [[SHUFFLE_LINE]] MMatchClient */	int					m_nAgentPort;

/* [[SHUFFLE_LINE]] MMatchClient */	MUID				m_uidAgentClient;
/* [[SHUFFLE_LINE]] MMatchClient */	char				m_szServerName[64];

/* [[SHUFFLE_LINE]] MMatchClient */	MUID				m_uidServer;
/* [[SHUFFLE_LINE]] MMatchClient */	int					m_nServerPort;
/* [[SHUFFLE_LINE]] MMatchClient */	MUID				m_uidStage;

/* [[SHUFFLE_LINE]] MMatchClient */	char				m_szServerIP[32];
/* [[SHUFFLE_LINE]] MMatchClient */	MClientSocket		m_AgentSocket;

/* [[SHUFFLE_LINE]] MMatchClient */	MUID				m_uidAgentServer;

protected:
	MMatchObjCacheMap	m_ObjCacheMap;
	MSafeUDP			m_SafeUDP;
	MMatchPeerInfoList	m_Peers;
	bool				m_bBridgePeerFlag;
	bool				m_bUDPTestProcess;
	MPacketCrypter		m_AgentPacketCrypter;		///< 에이전트 암호화 키
	MPacketCrypter		m_PeerPacketCrypter;		///< 클라이언트끼리 통신할때의 암호화 키


public:
	MCommand* MakeCmdFromTunnelingBlob(const MUID& uidSender, void* pBlob, int nBlobArrayCount);
	bool MakeTunnelingCommandBlob(MCommand* pWrappingCmd, MCommand* pSrcCmd);
	MPacketCrypter GetPeerPacketCrypter() { return m_PeerPacketCrypter; }

protected:
	bool GetAgentPeerFlag()				{ return m_bAgentPeerFlag; }
	void SetAgentPeerFlag(bool bVal)	{ m_bAgentPeerFlag = bVal; }
	int GetAgentPeerCount()				{ return m_nAgentPeerCount; }
	void SetAgentPeerCount(int nCount)	{ m_nAgentPeerCount = nCount; }
	void StartAgentPeerConnect();
	void CastAgentPeerConnect();
	void StartUDPTest(const MUID& uidChar);
	void InitPeerCrypt(const MUID& uidStage, unsigned int nChecksum);
protected:
	// tcp socket event
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

	virtual int OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp);
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);
	virtual int OnResponseMatchLogin(const MUID& uidServer, 
									 int nResult, 
									 const char* szServerName, 
		                             const MMatchServerMode nServerMode, 
									 const char* szAccountID, 
                                     const MMatchUserGradeID nUGradeID, 
                                     const MMatchPremiumGradeID nPGradeID,
									 const MUID& uidPlayer,
									 bool bEnabledSurvivalMode,
									 bool bEnabledDuelTournament,
//									 unsigned char* szRandomValue,
									 unsigned char* pbyGuidReqMsg);
	virtual void OnObjectCache(unsigned int nType, void* pBlob, int nCount);
	virtual void OnUDPTest(const MUID& uidChar);
	virtual void OnUDPTestReply(const MUID& uidChar);
	virtual void OnResponseAgentLogin();
	virtual void OnLocateAgentToClient(const MUID& uidAgent, char* szIP, int nPort, int nUDPPort);
	virtual void OnTunnelingTCP(const MUID& uidSender, void* pBlob, int nCount);
	virtual void OnTunnelingUDP(const MUID& uidSender, void* pBlob, int nCount);	
	virtual void OnAllowTunnelingTCP();
	virtual void OnAllowTunnelingUDP();	
	virtual void OnAgentConnected(const MUID& uidAgentServer, const MUID& uidAlloc);
	virtual void OnAgentError(int nError);

	void OutputLocalInfo(void);

	virtual void SendCommand(MCommand* pCommand);
	bool SendCommandToAgent(MCommand* pCommand);
	void SendCommandByTunneling(MCommand* pCommand);
	void ParseUDPPacket(char* pData,MPacketHeader* pPacketHeader,DWORD dwIP,unsigned int nPort);
public:
	void SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort);
/*
public :
	void SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort);
	*/

public:
	MMatchClient();
	virtual ~MMatchClient();

	bool Create(unsigned short nUDPPort);
	
	bool GetBridgePeerFlag()			{ return m_bBridgePeerFlag; }
	void SetBridgePeerFlag(bool bFlag)	{ m_bBridgePeerFlag = bFlag; }
	void AddPeer(MMatchPeerInfo* pPeerInfo);
	//bool DeletePeer(const char* szIP);
	bool DeletePeer(const MUID uid);
	MUID FindPeerUID(const DWORD dwIP, const int nPort);
	MMatchPeerInfo* FindPeer(const MUID& uidChar);
	void ClearPeers();
	void CastStageBridgePeer(const MUID& uidChar, const MUID& uidStage);	// UDP lost 대비해 Stage입장시 여러번 호출 필요

	bool GetUDPTestProcess()			{ return m_bUDPTestProcess; }
	void SetUDPTestProcess(bool bVal)	{ m_bUDPTestProcess = bVal; }
	void UpdateUDPTestProcess();
	void GetUDPTraffic(int* nSendTraffic, int* nRecvTraffic)	{ return m_SafeUDP.GetTraffic(nSendTraffic, nRecvTraffic); }

	void SetUDPPort(int nPort);
	__forceinline MUID GetServerUID() { return m_uidServer; }
	__forceinline MUID GetPlayerUID()	{ return m_uidPlayer; }
	__forceinline MUID GetChannelUID() { return m_uidChannel; }
	__forceinline MUID GetStageUID() { return m_uidStage; }
	virtual MUID GetSenderUIDBySocket(SOCKET socket);


	void SetServerAddr(const char* szIP, int nPort)	{ 
		strcpy(m_szServerIP,szIP), m_nServerPort = nPort; 
	}
	char* GetServerIP() { return m_szServerIP; }
	int GetServerPort() { return m_nServerPort; }
	void SetServerPeerPort(int nPeerPort) { m_nServerPeerPort = nPeerPort; }
	int GetServerPeerPort() { return m_nServerPeerPort; }

	MMatchPeerInfoList* GetPeers() { return &m_Peers; }	
	MSafeUDP* GetSafeUDP() { return &m_SafeUDP; }
	string GetObjName(const MUID& uid);
	MMatchObjCache* FindObjCache(const MUID& uid);
	void ReplaceObjCache(MMatchObjCache* pCache);
	void UpdateObjCache(MMatchObjCache* pCache);
	void RemoveObjCache(const MUID& uid);
	void ClearObjCaches();

	static bool UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize);

public:
	void SetAgentAddr(const char* szIP, int nPort)	{ 
		strcpy(m_szAgentIP,szIP), m_nAgentPort = nPort; 
	}
	char* GetAgentIP() { return m_szAgentIP; }
	int GetAgentPort() { return m_nAgentPort; }
	void SetAgentPeerPort(int nPeerPort) { m_nAgentPeerPort = nPeerPort; }
	int GetAgentPeerPort() { return m_nAgentPeerPort; }
	const MUID& GetAgentServerUID() { return m_uidAgentServer; }
	const MUID& GetAgentClientUID() { return m_uidAgentClient; }
	bool GetAllowTunneling() { return m_bAllowTunneling; }
	void SetAllowTunneling(bool bAllow) { m_bAllowTunneling = bAllow; }

	int AgentConnect(SOCKET* pSocket, char* szIP, int nPort);
	void AgentDisconnect();

	MMatchObjCacheMap* GetObjCacheMap() { return &m_ObjCacheMap; }
	MMatchServerMode GetServerMode()	{ return m_nServerMode; }
	const char* GetServerName()			{ return m_szServerName; }

	bool IsEnabledSurvivalMode() const	{ return m_bEnabledSurvivalMode; }
	bool IsEnabledDuelTournament() const { return m_bEnabledDuelTournament; }

	virtual bool IsUDPCommandValidationCheck(int nCommandID) = 0;
};

/* 
로컬에서 게임 2개 띄워놓고 테스트 하는 법
우선 ZoneServer를 띄운다.

Chat창에서 각각의 게임창마다
/con							 ,		/con
/peer.setport 10000				 ,		/peer.setport 10001 
/peer.addpeer 127.0.0.1 10001    ,		/peer.addpeer 127.0.0.1 10000 
이렇게 입력하고서 테스트 해야함.

왜냐하면 각 게임창마다 포트를 다르게 설정해야하기 때문..

*/

#endif