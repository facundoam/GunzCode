#include "stdafx.h"
#include "MMatchActiveTrapMgr.h"
#include "MBlobArray.h"


MMatchActiveTrap::MMatchActiveTrap()
	: m_vPosActivated(0, 0, 0)
{
	m_uidOwner.SetZero();
	m_nTrapItemId = 0;

	m_nTimeThrowed = 0;
	m_nLifeTime = 0;
	m_nTimeActivated = 0;
}

void MMatchActiveTrap::AddForcedEnteredPlayer(const MUID& uid)
{
	// 이 함수는 던져졌으나 아직 발동되지 않은 트랩을 난입자에게 나중에 알려주기 위해 사용된다.
	//_ASSERT(!IsActivated());

	int n = (int)m_vecUidForcedEntered.size();
	for (int i = 0; i < n; ++i)
		if (m_vecUidForcedEntered[i] == uid) return;

	m_vecUidForcedEntered.push_back(uid);
}


MMatchActiveTrapMgr::MMatchActiveTrapMgr()
{
	m_pStage = NULL;
}

MMatchActiveTrapMgr::~MMatchActiveTrapMgr()
{
	Destroy();
}

void MMatchActiveTrapMgr::Create(MMatchStage* pStage)
{
	m_pStage = pStage;
}

void MMatchActiveTrapMgr::Destroy()
{
	m_pStage = NULL;
	Clear();
}

void MMatchActiveTrapMgr::Clear()

{
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
		delete* it;
	m_listTrap.clear();
}

void MMatchActiveTrapMgr::AddThrowedTrap(const MUID& uidOwner, int nItemId)
{
	if (!m_pStage) return;
	if (!m_pStage->GetObj(uidOwner)) return;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemId);
	if (!pItemDesc) return;

	MMatchActiveTrap* pTrap = new MMatchActiveTrap;

	pTrap->m_nTimeThrowed = MGetMatchServer()->GetGlobalClockCount();
	pTrap->m_uidOwner = uidOwner;
	pTrap->m_nTrapItemId = nItemId;

	pTrap->m_nLifeTime = pItemDesc->m_nLifeTime.Ref();

	m_listTrap.push_back(pTrap);

	OutputDebugString("AddThrowedTrap\n");
}

void MMatchActiveTrapMgr::OnActivated(const MUID& uidOwner, int nItemId, const MVector3& vPos)
{
	if (!m_pStage) return;

	MMatchActiveTrap* pTrap;
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
	{
		pTrap = *it;
		if (pTrap->m_uidOwner == uidOwner &&
			pTrap->m_nTrapItemId == nItemId &&
			!pTrap->IsActivated())
		{
			pTrap->m_nTimeActivated = MGetMatchServer()->GetGlobalClockCount();
			pTrap->m_vPosActivated = vPos;

			OutputDebugString("OnActivated trap\n");

			// 발동되지 않은 트랩이 존재하던 시점에 난입했던 유저들이 있으면 이 트랩의 발동 사실을 알려준다
			RouteTrapActivationForForcedEnterd(pTrap);
			return;
		}
	}
}

void MMatchActiveTrapMgr::Update(unsigned long nClock)
{
	MMatchActiveTrap* pTrap;
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); )
	{
		pTrap = *it;

		// Check if the trap is activated
		if (pTrap->IsActivated())
		{
			// Ensure no underflow when comparing unsigned integers
			if (nClock >= pTrap->m_nTimeActivated && (nClock - pTrap->m_nTimeActivated > pTrap->m_nLifeTime))
			{
				// Erase the trap and delete the memory
				it = m_listTrap.erase(it);
				delete pTrap;  // Free the memory
				OutputDebugString("Trap deactivated\n");
				continue;
			}
		}
		else
		{
			// Ensure no underflow when comparing unsigned integers
			if (nClock >= pTrap->m_nTimeThrowed && (nClock - pTrap->m_nTimeThrowed > MAX_TRAP_THROWING_LIFE * 1000))
			{
				// Erase the trap and delete the memory
				it = m_listTrap.erase(it);
				delete pTrap;  // Free the memory
				OutputDebugString("Trap Removed without activation\n");
				continue;
			}
		}

		// Increment the iterator only if no trap is erased
		++it;
	}
}

void MMatchActiveTrapMgr::RouteAllTraps(MMatchObject* pObj)
{
	// 난입한 유저에게 현재 월드에 발동되어 있는 트랩 아이템들을 알려주기 위한 함수

	OutputDebugString("Trap RouteAllTrap to ForcedEntered\n");

	MMatchActiveTrap* pTrap;

	// 아직 발동되지 않은 트랩(던져서 날아가고 있는 중)은 이후 발동할 때 따로 알려줄 수 있도록 표시해둔다
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
	{
		pTrap = *it;
		if (!pTrap->IsActivated())
		{
			pTrap->AddForcedEnteredPlayer(pObj->GetUID());

			OutputDebugString("Trap RESERVE To NOTIFY AddForcedEnteredPlayer\n");
		}
	}

	// 발동되어 있는 트랩은 목록을 보내준다
	int num = 0;
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
		if ((*it)->IsActivated())
			++num;

	if (num <= 0) return;

	void* pTrapArray = MMakeBlobArray(sizeof(MTD_ActivatedTrap), num);

	MTD_ActivatedTrap* pNode;
	int nIndex = 0;
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
	{
		pTrap = *it;
		if (pTrap->IsActivated())
		{
			pNode = (MTD_ActivatedTrap*)MGetBlobArrayElement(pTrapArray, nIndex++);
			Make_MTDActivatedTrap(pNode, pTrap);
		}
		else
		{
			// 아직 발동되지 않은 트랩(던져서 날아가고 있는 중)은 이후 발동할 때 따로 알려줄 수 있도록 표시해둔다
			pTrap->AddForcedEnteredPlayer(pObj->GetUID());

			OutputDebugString("Trap RESERVE To NOTIFY AddForcedEnteredPlayer\n");
		}
	}

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterBlob(pTrapArray, MGetBlobArraySize(pTrapArray)));
	MEraseBlobArray(pTrapArray);

	MMatchServer::GetInstance()->RouteToListener(pObj, pCmd);
}

void MMatchActiveTrapMgr::RouteTrapActivationForForcedEnterd(MMatchActiveTrap* pTrap)
{
	OutputDebugString("Notify Trap activation to ForcedEnteredPlayer\n");

	if (!pTrap || !pTrap->IsActivated()) { return; }
	if (!m_pStage) return;

	int numTarget = (int)pTrap->m_vecUidForcedEntered.size();
	if (numTarget <= 0) return;

	void* pTrapArray = MMakeBlobArray(sizeof(MTD_ActivatedTrap), 1);

	MTD_ActivatedTrap* pNode = (MTD_ActivatedTrap*)MGetBlobArrayElement(pTrapArray, 0);
	Make_MTDActivatedTrap(pNode, pTrap);

	MCommand* pCommand = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST, MUID(0, 0));
	pCommand->AddParameter(new MCommandParameterBlob(pTrapArray, MGetBlobArraySize(pTrapArray)));

	MMatchObject* pObj;
	for (int i = 0; i < numTarget; ++i)
	{
		pObj = m_pStage->GetObj(pTrap->m_vecUidForcedEntered[i]);
		if (!pObj) continue;

		MCommand* pSendCmd = pCommand->Clone();
		MMatchServer::GetInstance()->RouteToListener(pObj, pSendCmd);
	}

	delete pCommand;
	MEraseBlobArray(pTrapArray);

	pTrap->m_vecUidForcedEntered.clear();
}