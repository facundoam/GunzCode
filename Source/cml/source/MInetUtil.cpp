#include "stdafx.h"
#include "Winsock2.h"
#include "MInetUtil.h"
#include <iostream>
#include <string>
#include <crtdbg.h>
#include <winsock2.h>  // or appropriate headers for your platform
#include <ws2tcpip.h>  // for inet_ntop

using std::string;

void MConvertCompactIP(char* szOut, const char* szInputDottedIP)
{
	in_addr addr;
	addr.S_un.S_addr = inet_addr(szInputDottedIP);
	sprintf(szOut, "%03u%03u%03u%03u", addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2,
		addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
}


void GetLocalIP(char* szOutIP, int nSize)
{
	if (szOutIP == nullptr || nSize <= 0)
		return;

	char szHostName[256];
	PHOSTENT pHostInfo;

	// Get the local host name
	if (gethostname(szHostName, sizeof(szHostName)) == 0)
	{
		// Get host information for the local machine
		if ((pHostInfo = gethostbyname(szHostName)) != nullptr)
		{
			// Check for the first address (IPv4)
			struct in_addr addr;
			memcpy(&addr, pHostInfo->h_addr_list[0], sizeof(struct in_addr));

			// Use inet_ntop for thread-safe IP address conversion
			if (inet_ntop(AF_INET, &addr, szOutIP, nSize) == nullptr)
			{
				// If the conversion failed, clear the output string
				szOutIP[0] = '\0';
			}
		}
	}
}


const bool MGetIPbyHostName(const string& strName, string& outIP)
{
	_ASSERT(0 == outIP.length());
	_ASSERT(0 < strName.length());
	_ASSERT(isalpha(strName[0]));

	if (0 == strName.length())
	{
		return false;
	}

	if (!isalpha(strName[0]))
	{
		outIP = strName;
		return true;
	}

	// WSAStartup을 먼저 해줘야 한다.
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return false;
	}

	struct hostent* pRemoteHost;
	pRemoteHost = gethostbyname(strName.c_str());
	if (NULL == pRemoteHost)
	{
		return false;
	}

	in_addr addr;
	addr.s_addr = *(u_long*)pRemoteHost->h_addr_list[0];

	outIP.clear();
	outIP = string(inet_ntoa(addr));

	return true;
}