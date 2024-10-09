#include "stdafx.h"
#include "CMLexicalAnalyzer.h"
#include <string.h>
#include <stdlib.h>

CMLexicalAnalyzer::CMLexicalAnalyzer(void) {}

CMLexicalAnalyzer::~CMLexicalAnalyzer(void)
{
	Destroy();
}

static bool StrTok(char* pToken, const char* pStr, const char* pSep)
{
	static char szTemp[256] = "";

	// If a new string is provided, copy it into the static buffer
	if (pStr != NULL) {
		strcpy_s(szTemp, sizeof(szTemp), pStr);
	}

	// Get the length of the current string
	int nLen = strlen(szTemp);
	if (nLen == 0) return false;

	// Get the length of the separator
	int nSepLen = strlen(pSep);

	// Loop through the string to find the first occurrence of a separator
	for (int i = 0; i < nLen; i++) {
		for (int j = 0; j < nSepLen; j++) {
			if (szTemp[i] == pSep[j]) {
				if (i == 0) {
					// Skip consecutive separators
					strcpy_s(szTemp, sizeof(szTemp), szTemp + 1);
					return StrTok(pToken, NULL, pSep); // Recursive call to continue
				}
				else {
					// Copy the token before the separator
					strncpy_s(pToken, 256, szTemp, i);
					pToken[i] = '\0'; // Null-terminate the token

					// Move the remaining string up
					strcpy_s(szTemp, sizeof(szTemp), szTemp + i + 1);
					return true;
				}
			}
		}
	}

	// If no separator is found, copy the entire remaining string as the last token
	strcpy_s(pToken, 256, szTemp);
	szTemp[0] = '\0'; // Clear the static buffer
	return true;
}

bool CMLexicalAnalyzer::Create(const char* pStr)
{
	// Token separators: space, comma, tab, and single quote
	char seps[] = " ,\t'";
	char szToken[256]; // Buffer for tokens

	// Initial tokenization
	if (StrTok(szToken, pStr, seps) == false) return true;

	while (1) {
		// Ignore specific separator tokens (space, comma, tab)
		if (szToken[0] == ' ') {
		}
		else if (szToken[0] == ',') {
		}
		else if (szToken[0] == '\t') {
		}
		else if (szToken[0] == '\'') {
			// Handle token enclosed in single quotes
			if (StrTok(szToken, NULL, "'") == false) return true;

			// Allocate memory for the token and copy it
			char* pAddToken = new char[strlen(szToken) + 1];
			strcpy_s(pAddToken, strlen(szToken) + 1, szToken); // Use strcpy_s for safer copying
			m_Tokens.Add(pAddToken);

			// Get the next token after the single quote
			if (StrTok(szToken, NULL, "'") == false) return true;
		}
		else {
			// Allocate memory for the token and copy it
			char* pAddToken = new char[strlen(szToken) + 1];
			strcpy_s(pAddToken, strlen(szToken) + 1, szToken); // Use strcpy_s for safer copying
			m_Tokens.Add(pAddToken);
		}

		// Get the next token
		if (StrTok(szToken, NULL, seps) == false) return true;
	}

	return true;
}

void CMLexicalAnalyzer::Destroy(void)
{
	for (int i = 0; i < m_Tokens.GetCount(); i++) {
		char* pToken = m_Tokens.Get(i);
		delete[] pToken;
	}
	m_Tokens.DeleteAll();
}

char* CMLexicalAnalyzer::GetByStr(int i)
{
	return m_Tokens.Get(i);
}

int CMLexicalAnalyzer::GetByInt(int i)
{
	return atoi(GetByStr(i));
}

long CMLexicalAnalyzer::GetByLong(int i)
{
	return atol(GetByStr(i));
}

float CMLexicalAnalyzer::GetByFloat(int i)
{
	return (float)atof(GetByStr(i));
}

int CMLexicalAnalyzer::GetCount(void)
{
	return m_Tokens.GetCount();
}

bool CMLexicalAnalyzer::IsNumber(int i)
{
	char* pStr = GetByStr(i);
	int len = strlen(pStr);
	for (int j = 0; j < len; j++) {
		if (!(pStr[j] >= '0' && pStr[j] < '9') || pStr[j] == '.')
			return false;
	}

	return true;
}
/*
char *CMLexicalAnalyzer::GetOrgStr(void)
{
	return m_pOriginal;
}
*/