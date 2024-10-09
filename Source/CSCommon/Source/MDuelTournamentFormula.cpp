#include "stdafx.h"
#include "MDuelTournamentFormula.h"

int MDuelTournamentFormula::Calc_WinnerTP(int nWinnerTP, int nLoserTP, bool isFinal)
{
	// Use powf() for float precision
	float fResult = 5.0f / (1.0f + powf(5.0f, float(nWinnerTP - nLoserTP) / 1000.0f));

	// Ensure fResult is at least 1
	if (fResult < 1.0f)
		fResult = 1.0f;

	// If final, return double the result
	if (isFinal)
		return static_cast<int>(fResult * 2.0f);
	else
		return static_cast<int>(fResult);
}

int MDuelTournamentFormula::Calc_LoserTP(int nWinnerGainTP, bool isFinal)
{
	float fWinnerGainTP = (float)nWinnerGainTP;
	if (isFinal)
		fWinnerGainTP /= 2.f;			// 결승인 경우 승자는 2배 보너스를 받으므로 원래 득점으로 되돌린 후

	return (int)(fWinnerGainTP / 2.f);	// 승자 득점의 절반만큼을 패자에게 감점
}