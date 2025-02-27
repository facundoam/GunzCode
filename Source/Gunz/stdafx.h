// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.
//

#pragma once
#define POINTER_64 __ptr64
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#pragma comment(lib, "legacy_stdio_definitions.lib")
// Windows 헤더 파일입니다.
#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>

#include <winsock2.h>
#include <mswsock.h>

#include <crtdbg.h>

#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#include <windows.h>

#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>

// C의 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <cstddef>
#include <comutil.h>
#include <stdio.h>


// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

#include "../CSCommon/MFeatureDefine.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//!! 이제는 여기에 하지 마시고 MFeatureDefine.h에서 define을 걸도록 합니다. 클라/서버 동시에 적용할 수 있습니다
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _QUEST		// 퀘스트 개발용 디파인

#define _QUEST_ITEM	// 퀘스트 아이템 개발용. //<- 현재 이걸 undef한다고 퀘스트아이템 코드를 제외할 수는 없게 되었습니다. (감싸지 않은 부분이 좀 있음)

#define _MONSTER_BIBLE 

//#define _GAMBLEITEM_TEST_LOG	// 겜블 아이템이 문제있을때 확인을 위해(평소에는 주석처리)

//#define _REPLAY_TEST_LOG // 리플레이시 특수아이템 사용 확인을 위해(평소는 주석으로)

//#define _LOCATOR

#if defined(LOCALE_NHNUSA)// || defined(_DEBUG)
	#define _MULTILANGUAGE	// 다중언어 지원 여부
#endif

#if defined(_DEBUG) || defined(_RELEASE) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA) || defined(LOCALE_JAPAN) || defined(LOCALE_BRAZIL)
	#define _DUELTOURNAMENT	// 듀얼토너먼트 지원 여부
#endif



// stl
#include <list>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <sstream>

// 외부 라이브러리
#include "d3d9.h"
#include "d3dx9math.h"

#include "fmod.h"

// cml
#include "MXml.h"
#include "MUtil.h"
#include "MDebug.h"
#include "MRTTI.h"
#include "MUID.h"
#include "MemPool.h"

// mint
#include "Mint.h"
#include "MWidget.h"
#include "MBitmap.h"
#include "MButton.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "MFrame.h"
#include "MPopupMenu.h"

// realspace2
#include "rtypes.h"
#include "RNameSpace.h"
#include "RTypes.h"
#include "RealSpace2.h"
#include "RBspObject.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "RMaterialList.h"
#include "RAnimationMgr.h"
#include "Mint4R2.h"

// cscommon
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchStage.h"
#include "MMatchItem.h"
#include "MMatchMap.h"
#include "MSafeUDP.h"
#include "MMatchClient.h"
#include "MGameClient.h"
#include "MMatchTransDataType.h"
#include "MErrorTable.h"
#include "Config.h"
#include "MSharedCommandTable.h"
#include "MObjectTypes.h"
#include "MMatchBuff.h"

// gunz global
#include "ZApplication.h"
#include "ZGlobal.h"
#include "ZMessages.h"
#include "ZStringResManager.h"
#include "ZGameInterface.h"
#include "ZCombatInterface.h"
#include "ZGame.h"
#include "ZGameClient.h"
#include "ZObject.h"
#include "ZIDLResource.h"
#include "ZInterfaceListener.h"
#include "ZColorTable.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"
#include "ZNetRepository.h"
#include "ZItem.h"
#include "ZItemDesc.h"
#include "ZPost.h"
#include "ZSoundEngine.h"
#include "ZSoundFMod.h"
#include "ZCamera.h"
#include "ZCharacter.h"
#include "ZActor.h"