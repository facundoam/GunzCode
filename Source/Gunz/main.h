#ifndef _MAIN_H
#define _MAIN_H

//#pragma once

#pragma warning(disable:4099)

#ifdef _DEBUG
#pragma comment(lib,"cml.lib")
#pragma comment(lib,"mint2.lib")
#pragma comment(lib,"RealSound.lib")
#pragma comment(lib,"RealSpace2.lib")
#pragma comment(lib,"CSCommonE.lib")
#pragma comment(lib,"SafeUDP.lib")
#pragma comment(lib, "dxerr9.lib")

#else
#pragma comment(lib,"cml.lib")
#pragma comment(lib,"mint2.lib")
#pragma comment(lib,"RealSound.lib")
#pragma comment(lib,"RealSpace2.lib")
#pragma comment(lib,"CSCommonE.lib")
#pragma comment(lib,"SafeUDP.lib")
#pragma comment(lib, "dxerr9.lib")

#endif


#pragma comment(lib,"winmm.lib")



#endif