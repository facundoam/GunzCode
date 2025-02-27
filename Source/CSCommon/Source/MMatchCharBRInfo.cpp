#include "stdafx.h"
#include "MMatchCharBRInfo.h"

#include "MMatchGlobal.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchCharBRInfo::MMatchCharBRInfo()
	: m_bCheckSkip(true), m_nLastCheckTime(0), m_nLastUpdateDBTime(0)
{
	SetBRInfo(-1, 0, 0, 0, 0);
}

MMatchCharBRInfo::MMatchCharBRInfo(int nBRID, int nBRTID, unsigned long int nBattleTime, int nRewardCount, int nKillCount)
	: m_bCheckSkip(true), m_nLastCheckTime(0), m_nLastUpdateDBTime(0)
{
	SetBRInfo(nBRID, nBRTID, nBattleTime, nRewardCount, nKillCount);
}

void MMatchCharBRInfo::ResetInfo()
{
	m_nBattleTime = 0;
	m_nKillCount = 0;
}

void MMatchCharBRInfo::SetBRInfo(int nBRID, int nBRTID, unsigned long int nBattleTime, int nRewardCount, int nKillCount)
{
	m_nBRID = nBRID;
	m_nBRTID = nBRTID;

	m_nBattleTime = nBattleTime;

	m_nRewardCount = nRewardCount;
	m_nKillCount = nKillCount;
}

bool MMatchCharBRInfo::IsExpired(int nBRTID)
{
	if (m_nBRTID == nBRTID) 		return false;
	else if (m_nBRTID < nBRTID)	return true;

	return false;
}

bool MMatchCharBRInfo::IsNeedUpdateDB(unsigned long int nTick)
{
	if (nTick - GetLastUpdateDBTime() > CYCLE_CHAR_BATTLE_TIME_UPDATE_DB)
	{
		SetLastUpdateDBTime(nTick);
		return true;
	}

	return false;
}

BRRESULT MMatchCharBRInfo::CheckBattleTimeReward(unsigned long int nTick, MMatchBRDescription* pDesc)
{
	// Ensure pDesc is not nullptr
	if (pDesc == nullptr)
	{
		return BRRESULT_NO_REWARD;  // Handle error or log appropriately
	}

	// Check for unsigned underflow
	unsigned long int nLastCheckTime = GetLastCheckTime();
	if (nTick < nLastCheckTime)
	{
		return BRRESULT_NO_REWARD;  // Handle time synchronization issues, or reset
	}

	unsigned long int nAddedTime = nTick - nLastCheckTime;
	if (nAddedTime > CYCLE_CHAR_BATTLE_TIME_CHECK)
	{
		SetLastCheckTime(nTick);
		m_nBattleTime += nAddedTime;

		// Check if the conditions for rewards are met
		if (pDesc->GetRewardkillCount() > m_nKillCount)
			return BRRESULT_NO_REWARD;

		if (pDesc->GetRewardMinutePeriod() * MINUTE_PERIOD_UNIT > m_nBattleTime)
			return BRRESULT_NO_REWARD;

		// Return appropriate result based on reward count
		if (pDesc->GetRewardCount() == 0)
		{
			return BRRESULT_DO_REWARD;
		}
		else if (m_nRewardCount < pDesc->GetRewardCount())
		{
			return BRRESULT_DO_REWARD;
		}
		else
		{
			return BRRESULT_RESET_INFO;
		}
	}

	return BRRESULT_NO_REWARD;
}