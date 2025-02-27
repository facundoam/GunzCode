#ifndef _ZCHARACTER_H
#define _ZCHARACTER_H

//#pragma	once

#include "MRTTI.h"
#include "ZCharacterObject.h"
//#include "ZActor.h"
#include "MUID.h"
#include "RTypes.h"
#include "RPathFinder.h"
#include "RVisualMeshMgr.h"

#include "MObjectTypes.h"
//#include "MObjectCharacter.h"
#include "ZItem.h"
#include "ZCharacterItem.h"
#include "ZCharacterBuff.h"

#include "MMatchObject.h"
#include "RCharCloth.h"
#include "ZFile.h"
#include "Mempool.h"

#include "ZModule_HPAP.h"

#include <list>
#include <string>

using namespace std;

_USING_NAMESPACE_REALSPACE2

#define MAX_SPEED			1000.f			// 최대속도..
#define RUN_SPEED			630.f			// 달리는 속도
#define BACK_SPEED			450.f			// 뒤나 옆으로 갈때 속도
#define ACCEL_SPEED			7000.f			// 가속도
#define STOP_SPEED			3000.f			// 아무키도 안눌렀을때 감속도..
#define STOP_FORMAX_SPEED	7100.f			// 달리는 속도 이상으로 올라갔을때 빨리 감속하는 속도

#define CHARACTER_RADIUS	35.f		// 캐릭터 충돌 반지름
#define CHARACTER_HEIGHT	180.0f		// 캐릭터 충돌 높이

#define ARRIVAL_TOLER		5.f

class ZShadow;

struct ZANIMATIONINFO {
	char *Name;
	bool bEnableCancel;		// 캔슬 가능한지
	bool bLoop;				// 반복 되는 동작
	bool bMove;				// 움직임이 포함된 애니메이션
	bool bContinuos;		// 포함된 움직임이 시작부터 연결되어있는지.
};

struct ZFACECOSTUME
{
	char* szMaleMeshName;
	char* szFemaleMeshName;
};


enum ZC_SKILL {

	ZC_SKILL_NONE = 0,

	ZC_SKILL_UPPERCUT,
	ZC_SKILL_SPLASHSHOT,
	ZC_SKILL_DASH,
	ZC_SKILL_CHARGEDSHOT,

	ZC_SKILL_END
};


enum ZC_DIE_ACTION
{
	ZC_DIE_ACTION_RIFLE = 0,
	ZC_DIE_ACTION_KNIFE,
	ZC_DIE_ACTION_SHOTGUN,
	ZC_DIE_ACTION_ROCKET,

	ZC_DIE_ACTION_END
};

enum ZC_SPMOTION_TYPE {

	ZC_SPMOTION_TAUNT = 0,
	ZC_SPMOTION_BOW ,
	ZC_SPMOTION_WAVE ,
	ZC_SPMOTION_LAUGH ,
	ZC_SPMOTION_CRY ,
	ZC_SPMOTION_DANCE ,

	ZC_SPMOTION_END
};

enum ZC_WEAPON_SLOT_TYPE {

	ZC_SLOT_MELEE_WEAPON = 0,
	ZC_SLOT_PRIMARY_WEAPON,
	ZC_SLOT_SECONDARY_WEAPON,
	ZC_SLOT_ITEM1,
	ZC_SLOT_ITEM2,

	ZC_SLOT_END,
};

enum ZC_SHOT_SP_TYPE {
	ZC_WEAPON_SP_NONE = 0,

	// grenade type
	ZC_WEAPON_SP_GRENADE,
	ZC_WEAPON_SP_ROCKET,
	ZC_WEAPON_SP_FLASHBANG,
	ZC_WEAPON_SP_SMOKE,
	ZC_WEAPON_SP_TEAR_GAS,

	ZC_WEAPON_SP_ITEMKIT,	// medikit, repairkit, bulletkit 등등

	ZC_WEAPON_SP_POTION,
	ZC_WEAPON_SP_TRAP,
	ZC_WEAPON_SP_DYNAMITE,
	ZC_WEAPON_SP_END,
};

enum ZSTUNTYPE {
	ZST_NONE	=	-1,
	ZST_DAMAGE1	=	0,
	ZST_DAMAGE2,
	ZST_SLASH,			// 강베기 스턴
	ZST_BLOCKED,		// 칼 막혔을때 스턴
	ZST_LIGHTNING,		// 인챈트중 Lightning
	ZST_LOOP,			// 스킬중 root 속성
};


class ZSlot {
public:
	ZSlot() {
		m_WeaponID = 0;
	}

	int m_WeaponID;
};

/// 캐릭터 속성 - 이 값은 변하지 않는다.
struct ZCharacterProperty_Old
{
	char		szName[MATCHOBJECT_NAME_LENGTH];
	char		szClanName[CLAN_NAME_LENGTH];
	MMatchSex	nSex;
	int			nHair;
	int			nFace;
	int			nLevel;
	float		fMaxHP;
	float		fMaxAP;
	int			nMoveSpeed;
	int			nWeight;
	int			nMaxWeight;
	int			nSafeFall;
	ZCharacterProperty_Old() :	nSex(MMS_MALE),
							nHair(0),
							nFace(0),
							nLevel(1),
							fMaxHP(1000.f), 
							fMaxAP(1000.f), 
							nMoveSpeed(100), 
							nWeight(0), 
							nMaxWeight(100), 
							nSafeFall(100)
							{ 
								szName[0] = 0;
								szClanName[0] = 0;
							}
	void SetName(const char* name) {
		strcpy(szName, name);
	}

	void SetClanName(const char* name) {
		strcpy(szClanName, name);
	}
};

struct ZCharacterProperty_CharClanName
{
	char		szName[MATCHOBJECT_NAME_LENGTH];
	char		szClanName[CLAN_NAME_LENGTH];
};
struct ZCharacterProperty
{
	MProtectValue<ZCharacterProperty_CharClanName> nameCharClan;
	MMatchSex	nSex;
	int			nHair;
	int			nFace;
	int			nLevel;
	MProtectValue<float>		fMaxHP;		// 이 값을 바꾸고 자살->리스폰해서 HP를 뻥튀기하는 방식의 해킹
	MProtectValue<float>		fMaxAP;
	ZCharacterProperty() :	nSex(MMS_MALE),
		nHair(0),
		nFace(0),
		nLevel(1)
	{ 
		nameCharClan.Ref().szName[0] = 0;
		nameCharClan.Ref().szClanName[0] = 0;
		nameCharClan.MakeCrc();

		//fMaxHP.Set_MakeCrc(1000.f);
		//fMaxAP.Set_MakeCrc(1000.f);
		fMaxHP.Set_MakeCrc(DEFAULT_CHAR_HP);
		fMaxAP.Set_MakeCrc(DEFAULT_CHAR_AP);
	}
	void SetName(const char* name)	   { nameCharClan.CheckCrc(); strcpy(nameCharClan.Ref().szName, name);	   nameCharClan.MakeCrc(); }
	void SetClanName(const char* name) { nameCharClan.CheckCrc(); strcpy(nameCharClan.Ref().szClanName, name); nameCharClan.MakeCrc(); }
	const char* GetName() { return nameCharClan.Ref().szName; }
	const char* GetClanName() { return nameCharClan.Ref().szClanName; }

	void CopyToOldStruct(ZCharacterProperty_Old& old)	// 리플레이 저장용
	{
		memcpy(old.szName, nameCharClan.Ref().szName, MATCHOBJECT_NAME_LENGTH);
		memcpy(old.szClanName, nameCharClan.Ref().szClanName, CLAN_NAME_LENGTH);
		
		old.nSex = nSex;
		old.nHair = nHair;
		old.nFace = nFace;
		old.nLevel = nLevel;
		old.fMaxHP = fMaxHP.Ref();		// TodoH(상) - 리플레이 관련..
		old.fMaxAP = fMaxAP.Ref();		// TodoH(상) - 리플레이 관련..
		old.nMoveSpeed = 100;
		old.nWeight = 0;
		old.nMaxWeight = 100;
		old.nSafeFall = 100;
	}

	void CopyFromOldStruct(ZCharacterProperty_Old& old)	// 리플레이 재생용
	{
		memcpy(nameCharClan.Ref().szName, old.szName, MATCHOBJECT_NAME_LENGTH);
		memcpy(nameCharClan.Ref().szClanName, old.szClanName, CLAN_NAME_LENGTH);
		nameCharClan.MakeCrc();

		nSex = old.nSex;
		nHair = old.nHair;
		nFace = old.nFace;
		nLevel = old.nLevel;
		fMaxHP.Set_MakeCrc(old.fMaxHP);		// TodoH(상) - 리플레이 관련..
		fMaxAP.Set_MakeCrc(old.fMaxAP);		// TodoH(상) - 리플레이 관련..
	}
};

/// 캐릭터 상태값
struct ZCharacterStatus
{	// 수정시 기존 리플레이 로딩 고려해야합니다
	int			nLife;
	int			nKills;
	int			nDeaths;
	int			nLoadingPercent;	// 처음 방에 들어올때 로딩이 다 되었는지의 퍼센트 100이 되면 로딩이 다된것
	int			nCombo;
	int			nMaxCombo;
	int			nAllKill;
	int			nExcellent;
	int			nFantastic;
	int			nHeadShot;
	int			nUnbelievable;
	int			nExp;
	int			nDamageCaused;
	int			nDamageTaken;

	ZCharacterStatus() :	
							nLife(10),
							nKills(0),
							nDeaths(0),
							nLoadingPercent(0),
							nCombo(0),
							nMaxCombo(0),
							nAllKill(0),
							nExcellent(0),
							nFantastic(0),
							nHeadShot(0),
							nUnbelievable(0),
							nExp(0),							
							nDamageCaused(0),
							nDamageTaken(0)

							{  }

	void AddKills(int nAddedKills = 1) { nKills += nAddedKills; }
	void AddDeaths(int nAddedDeaths = 1) { nDeaths += nAddedDeaths;  }
	void AddExp(int _nExp=1) { nExp += _nExp; }
};

// 이것은 캐릭터끼리 주고받는 데이터로 나중에 투표 판정의 근거가 된다.
/*
struct ZHPItem {
	MUID muid;
	float fHP;
};
*/

//struct ZHPInfoItem : public CMemPoolSm<ZHPInfoItem>{
//	ZHPInfoItem()	{ pHPTable=NULL; }
//	~ZHPInfoItem()	{ if(pHPTable) delete pHPTable; }
//	
//	int		nCount;
//	ZHPItem *pHPTable;
//};


//class ZHPInfoHistory : public list<ZHPInfoItem*> {
//};

#define CHARACTER_ICON_DELAY		2.f
#define CHARACTER_ICON_FADE_DELAY	.2f
#define CHARACTER_ICON_SIZE			32.f		// 아이콘 크기 (640x480기준)

class ZModule_HPAP;
class ZModule_QuestStatus;

// 1bit 패킹
struct ZCharaterStatusBitPacking { 
	union {
		struct {
			bool	m_bLand:1;				// 지금 발을 땅에 대고있는지..
			bool	m_bWallJump:1;			// 벽점프 중인지
			bool	m_bJumpUp:1;			// 점프올라가는중
			bool	m_bJumpDown:1;			// 내려가는중
			bool	m_bWallJump2:1;			// 이건 walljump 후에 착지시 두번째 튕겨져 나오는 점프..
			bool	m_bTumble:1;			// 덤블링 중
			bool	m_bBlast:1;				// 띄워짐당할때 ( 올라갈때 )
			bool	m_bBlastFall:1;			// 띄워져서 떨어질때
			bool	m_bBlastDrop:1;			// 떨어지다 땅에 튕길때
			bool	m_bBlastStand:1;		// 일어날때
			bool	m_bBlastAirmove:1;		// 공중회전후 착지
			bool	m_bSpMotion:1;
			bool	m_bCommander:1;			///< 대장
		//	bool	m_bCharging:1;			// 힘모으고 있는중
		//	bool	m_bCharged:1;			// 힘모인상태
			bool	m_bLostConEffect:1;		// 네트웍 응답이 없을때 머리에 뜨는 이펙트가 나와야 하는지.
			bool	m_bChatEffect:1;		// 채팅시 머리에 뜨는 이펙트가 나와야 하는지.
			bool	m_bBackMoving:1;		// 뒤로 이동할때

			bool	m_bAdminHide:1;					///< admin hide 되어있는지..
			bool	m_bDie:1;						///< 죽었는지 체크
			bool	m_bStylishShoted:1;				///< 마지막으로 쏜게 스타일리쉬 했는지 체크
			bool	m_bFallingToNarak:1;			///< 나락으로 떨어지고 있는지 여부
			bool	m_bStun:1;						///< stun ..움직일수없게된상태.
			bool	m_bDamaged:1;					///< 데미지 여부
				
			bool	m_bPlayDone:1;				// 애니메이션 플레이가 다 되었는지. 다음동작으로 넘어가는 기준
			bool	m_bPlayDone_upper:1;		// 상체 애니메이션 플레이가 다 되었는지. 다음동작으로 넘어가는 기준
			bool	m_bIsLowModel:1;
			bool	m_bTagger:1;					///< 술래
		};

		DWORD dwFlagsPublic;
	}; // 패킹 끝
};

struct ZUserAndClanName
{
	char m_szUserName[MATCHOBJECT_NAME_LENGTH];			///< 유져이름(운영자는 '운영자')
	char m_szUserAndClanName[MATCHOBJECT_NAME_LENGTH];  ///< 유저이름(클랜이름) 요걸 스캔해서 애들이 메모리 핵을 쓰니까 요걸 숨겨버리자..
};


/// 캐릭터 클래스
class ZCharacter : public ZCharacterObject
{
	MDeclareRTTI;
	//friend class ZCharacterManager;
private:
protected:

	// 모듈들. 한번 생성되고 소멸될때 같이 지운다
	ZModule_HPAP			*m_pModule_HPAP;
	ZModule_QuestStatus		*m_pModule_QuestStatus;
	ZModule_Resistance		*m_pModule_Resistance;
	ZModule_FireDamage		*m_pModule_FireDamage;
	ZModule_ColdDamage		*m_pModule_ColdDamage;
	ZModule_PoisonDamage	*m_pModule_PoisonDamage;
	ZModule_LightningDamage	*m_pModule_LightningDamage;
	ZModule_HealOverTime	*m_pModule_HealOverTime;
	

	ZCharacterProperty					m_Property;		///< HP 등의 캐릭터 속성
	MProtectValue<ZCharacterStatus>		m_Status;		///< 플레이어 상태값

	MProtectValue<MTD_CharInfo>			m_MInitialInfo;		///< 캐릭터 초기정보


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 게임 안에서의 케릭터 버프 관련 정보 및 함수
protected:	
	//버프정보임시주석 ZCharacterBuff m_CharacterBuff;				///< 적용되고 있는 버프

	float m_fPreMaxHP;
	float m_fPreMaxAP;

public:
	//버프정보임시주석 
	/*
	void SetCharacterBuff(MTD_CharBuffInfo* pCharBuffInfo);
	ZCharacterBuff* GetCharacterBuff() { return &m_CharacterBuff;}
	*/

	void ApplyBuffEffect();

	float GetMaxHP();
	float GetMaxAP();
	float GetHP();
	float GetAP();
	void InitAccumulationDamage();
	float GetAccumulationDamage();
	void EnableAccumulationDamage(bool bAccumulationDamage);

	__forceinline void SetMaxHP(float nMaxHP) { m_pModule_HPAP->SetMaxHP(nMaxHP); }
	__forceinline void SetMaxAP(float nMaxAP) { m_pModule_HPAP->SetMaxAP(nMaxAP); }

	__forceinline void SetHP(float nHP)	{ m_pModule_HPAP->SetHP(nHP); }
	__forceinline void SetAP(float nAP) { m_pModule_HPAP->SetAP(nAP); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	MProtectValue<ZUserAndClanName>*  m_pMUserAndClanName;  ///< 캐릭명,클랜명

	struct KillInfo {
		int			m_nKillsThisRound;				///< 이번라운드에서의 kills ( unbelievable 판정)
		float		m_fLastKillTime;				///< 마지막에 죽인 시간 (excellent)를 표시하기 위함
	};
	MProtectValue<KillInfo> m_killInfo;

	struct DamageInfo {
		DWORD			m_dwLastDamagedTime;		// 마지막으로 공격 받은 시간
		ZSTUNTYPE		m_nStunType;				///< 맞는 애니메이션 종류.. 2:마지막타격,4:lightning,5:루프
		ZDAMAGETYPE		m_LastDamageType;			///< 마지막 데미지 타입		
		MMatchWeaponType m_LastDamageWeapon;		///< 마지막 데미지 무기..
		rvector			m_LastDamageDir;			///< 마지막 데미지 방향 ( 죽는 모션을 결정 )
		float			m_LastDamageDot;
		float			m_LastDamageDistance;

		MUID			m_LastThrower;				///< 마지막 띄운 사람
		float			m_tmLastThrowClear;			///< 마지막 띄운 사람 잊어도 되는시간
	};
	MProtectValue<DamageInfo> m_damageInfo;

	int	m_nWhichFootSound;	///< 발소리를 번갈아 내기위해 어느 발인지 저장한 변수

	MProtectValue<DWORD>* m_pMdwInvincibleStartTime;		// 무적의 시작 시간
	MProtectValue<DWORD>* m_pMdwInvincibleDuration;		// 무적의 지속시간

	virtual void UpdateSound();

	void InitMesh();	///< 캐릭터 파츠 등의 메쉬정보 세팅. InitCharInfo에서 호출	


	void InitProperties();

//	float m_fIconStartTime[ZCI_END];	///< 머리위에 뜨는 아이콘들

	void CheckLostConn();
	virtual void OnLevelDown();
	virtual void OnLevelUp();
	virtual void OnDraw();
//	virtual void RegisterModules();
	virtual void	OnDie();

	void ShiftFugitiveValues();

public:
	float	m_fLastValidTime;		// Dead Reckoning에 필요한 변수 -> 지금 코드에서 필요없어보임
	DWORD		m_dwIsValidTime;	//디버그 레지스터 해킹 방어를 위한 타임 체크..강베기쪽..

//	float	m_fDistToFloor;			// 바닥까지의 거리
//	rplane	m_FloorPlane;			// 바닥 평면의 방정식
//	float	m_fFallHeight;			// 낙하가 시작된 시점

	MProtectValue<ZCharaterStatusBitPacking> m_dwStatusBitPackingValue;	// 얘는 인간적으로 crc체크까진 못하겠다...;;

	//mmemory proxy
	MProtectValue<bool>* m_bCharged;
	MProtectValue<bool>* m_bCharging;

	

	MProtectValue<float>	m_fChargedFreeTime;		// 힘모인게 풀리는 시간
	MProtectValue<int>		m_nWallJumpDir;			// 벽점프하는 방향
	MProtectValue<int>		m_nBlastType;			// 단도계열추가~


	ZC_STATE_LOWER	m_SpMotion;

	
	/*
	bool	m_bClimb;				// 턱등에 올라가고있는 경우
	rvector	m_ClimbDir;				// 올라가는 방향
	float	m_fClimbZ;				// 올라가기 시작한 높이
	rplane	m_ClimbPlane;
	*/

//	bool	m_bRendered;				///< 이전프레임에서 화면에 나왔는지

	/////// 네트웍에서 임시로 옷 갈아 입기 위한 현재 선택된 파츠정보
	//int m_t_parts[6];	//남자
	//int m_t_parts2[6];	//여자
	
	
	// rvector		m_vProxyPosition, m_vProxyDirection;	///< 이동값이 있는 애니메이션의 렌더링 위치를 위한 변수
	
//	ZHPInfoHistory		m_HPHistory;		///< 투표를 위해 몇초간의 데이터를 가지고있는다

	ZCharacter();
	virtual ~ZCharacter();

	virtual bool Create(MTD_CharInfo* pCharInfo/*, MTD_CharBuffInfo* pCharBuffInfo*/);//버프정보임시주석 
	virtual void Destroy();
	
	void InitMeshParts();
	
	void EmptyHistory();

	rvector m_TargetDir;
	rvector m_DirectionLower,m_DirectionUpper;

	// 이 변수들은 이동속도 해킹대상이 됨
	MProtectValue<rvector> m_RealPositionBefore;			// 애니메이션의 움직임을 추적하기 위한 변수
	MProtectValue<rvector> m_AnimationPositionDiff;
	MProtectValue<rvector> m_Accel;


	MProtectValue<ZC_STATE_UPPER>	m_AniState_Upper;		// 상체 애니메이션 상태
	MProtectValue<ZC_STATE_LOWER>	m_AniState_Lower;		// 하체 애니메이션 상태 (기본)
	ZANIMATIONINFO *m_pAnimationInfo_Upper,*m_pAnimationInfo_Lower;

	void AddIcon(int nIcon);
//	float GetIconStartTime(int nIcon);

	MProtectValue<int>				m_nVMID;	// VisualMesh ID
	//MUID	m_UID;		// 서버에서 부여한 캐릭터의 UID
	MProtectValue<MMatchTeam>		m_nTeamID;	// Team ID

	MProtectValue<MCharacterMoveMode>		m_nMoveMode;
	MProtectValue<MCharacterMode>			m_nMode;
	MProtectValue<MCharacterState>			m_nState;

//	RVisualMesh*			m_pVMesh;

//	float	m_fLastAdjustedTime;
	MProtectValue<float>	m_fAttack1Ratio;//칼질등의 경우 첫번째비율을 나중타에도 적용한다..
//	rvector m_AdjustedNormal;

	/*
	bool	m_bAutoDir;

	bool	m_bLeftMoving,m_bRightMoving;
	bool	m_bForwardMoving;
	*/

//	float	m_fLastUpdateTime;
	float	m_fLastReceivedTime;	// basicinfo 데이터를 마지막 받은 시간

	float	m_fTimeOffset;				// 나와 이 캐릭터의 시간차이
	float	m_fAccumulatedTimeError;	// 현재시간의 누적된 오차
	int		m_nTimeErrorCount;			// 현재시간의 오차가 누적된 회수


	float	m_fGlobalHP;			// 다른사람들이 볼때 이캐릭터의 HP의 평균.. 투표를 위함.
	int		m_nReceiveHPCount;		// 평균내기위한...

	/*
	float	m_fAveragePingTime;				// 일정시간 평균 네트웍 지연 시간

	// 글로벌 시간과의 차이를 누적한다. 이 숫자가 점점 커지는 유저는 스피드핵이 의심된다.
	#define TIME_ERROR_CORRECTION_PERIOD	20

	int		m_nTimeErrorCount;
	float	m_TimeErrors[TIME_ERROR_CORRECTION_PERIOD];
	float	m_fAccumulatedTimeError;

	list<float> m_PingData;			// 몇개의 핑 타임을 가지고 평균을 낸다.
	*/

//	DWORD m_dwBackUpTime;

	// 무기 발사속도의 이상유무를 검출한다.
	int		m_nLastShotItemID;
	float	m_fLastShotTime;

	inline void	SetInvincibleTime(int nDuration);
	inline bool	isInvincible();

	bool IsMan();

	virtual void  OnUpdate(float fDelta);

	//버프정보임시주석 virtual void  UpdateBuff();
	virtual void  UpdateSpeed();
	virtual float GetMoveSpeedRatio();

	virtual void UpdateVelocity(float fDelta);	// 적당한 속도로 감속
	virtual void UpdateHeight(float fDelta);		// 높이와 폴링 데미지를 검사.
	virtual void UpdateMotion(float fDelta=0.f);	// 허리돌리기등의 애니메이션 상태관련
	virtual void UpdateAnimation();

	int  GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc);

	void UpdateLoadAnimation();

	void Stop();
	//void DrawForce(bool bDrawShadow);	

	void CheckDrawWeaponTrack();
	void UpdateSpWeapon();

	void SetAnimation(char *AnimationName,bool bEnableCancel,int tick);
	void SetAnimation(RAniMode mode,char *AnimationName,bool bEnableCancel,int tick);

	void SetAnimationLower(ZC_STATE_LOWER nAni);
	void SetAnimationUpper(ZC_STATE_UPPER nAni);
	
	ZC_STATE_LOWER GetStateLower() { return m_AniState_Lower.Ref(); }
	ZC_STATE_UPPER GetStateUpper() { return m_AniState_Upper.Ref(); }

	bool IsUpperPlayDone()	{ return m_dwStatusBitPackingValue.Ref().m_bPlayDone_upper; }

	bool IsMoveAnimation();		// 움직임이 포함된 애니메이션인가 ?

//	bool IsRendered()		{ return m_bRendered; }

	bool IsTeam(ZCharacter* pChar);

	bool IsRunWall();
	bool IsMeleeWeapon();
	virtual bool IsCollideable();

//	void SetAnimationForce(ZC_STATE nState, ZC_STATE_SUB nStateSub) {}

	void SetTargetDir(rvector vDir); 

//	bool Pick(int x,int y,RPickInfo* pInfo);
//	bool Pick(int x,int y,rvector* v,float* f);
	virtual bool Pick(rvector& pos,rvector& dir, RPickInfo* pInfo = NULL);

//	bool Move(rvector &diff);

	void OnSetSlot(int nSlot,int WeaponID);
	void OnChangeSlot(int nSlot);

	virtual void OnChangeWeapon(char* WeaponModelName);
	void OnChangeParts(RMeshPartsType type,int PartsID);
	void OnAttack(int type,rvector& pos);
//	void OnHeal(ZCharacter* pAttacter,int type,int heal);
	void OnShot();

	void ChangeWeapon(MMatchCharItemParts nParts);

	int GetLastShotItemID() const { return m_nLastShotItemID; }
	float GetLastShotTime() const { return m_fLastShotTime; }
	bool CheckValidShotTime(int nItemID, float fTime, ZItem* pItem);
	void UpdateValidShotTime(int nItemID, float fTime)	
	{ 
		m_nLastShotItemID = nItemID;
		m_fLastShotTime = fTime;
	}

	bool IsDie() { return m_dwStatusBitPackingValue.Ref().m_bDie; }
	void ForceDie() 
	{
		SetHP(0); 
		m_dwStatusBitPackingValue.Ref().m_bDie = true; 
	}		// 이것은 그냥 죽은 상태로 만들기 위할때 사용

	void SetAccel(rvector& accel) { m_Accel.Set_CheckCrc(accel); }
	virtual void SetDirection(rvector& dir);

	struct SlotInfo
	{
		int		m_nSelectSlot;
		ZSlot	m_Slot[ZC_SLOT_END];
	};
	MProtectValue<SlotInfo>	m_slotInfo;

	MProtectValue<ZCharacterStatus>& GetStatus()	{ return m_Status; }

	// Character Property
	ZCharacterProperty* GetProperty() { return &m_Property; }

	MMatchUserGradeID GetUserGrade()	{ return m_MInitialInfo.Ref().nUGradeID; }
	unsigned int GetClanID()	{ return m_MInitialInfo.Ref().nClanCLID; }

	void SetName(const char* szName) { m_Property.SetName(szName); }

	const char *GetUserName()			{ return m_pMUserAndClanName->Ref().m_szUserName;	}		// 운영자는 처리됨
	const char *GetUserAndClanName()	{ return m_pMUserAndClanName->Ref().m_szUserAndClanName; }	// 운영자는 클랜표시 안함
	bool IsAdminName();
	bool IsAdminHide()			{ return m_dwStatusBitPackingValue.Ref().m_bAdminHide;	}
	void SetAdminHide(bool bHide) { m_dwStatusBitPackingValue.Ref().m_bAdminHide = bHide; }
	
//	void SetMoveSpeed(int nMoveSpeed) { m_Property.nMoveSpeed = nMoveSpeed; }
//	void SetWeight(int nWeight) { m_Property.nWeight = nWeight; }
//	void SetMaxWeight(int nMaxWeight) { m_Property.nMaxWeight = nMaxWeight; }
//	void SetSafeFall(int nSafeFall) { m_Property.nSafeFall = nSafeFall; }

	int GetKils() { return GetStatus().Ref().nKills; }

	bool CheckDrawGrenade();

//	int GetWeaponEffectType();
//	float GetCurrentWeaponRange();
//	float GetMeleeWeaponRange();

	bool GetStylishShoted() { return m_dwStatusBitPackingValue.Ref().m_bStylishShoted; }
	void UpdateStylishShoted();
	
	MUID GetLastAttacker() { return m_pModule_HPAP->GetLastAttacker(); }
	void SetLastAttacker(MUID uid) { m_pModule_HPAP->SetLastAttacker(uid); }
	ZDAMAGETYPE GetLastDamageType() { return m_damageInfo.Ref().m_LastDamageType; }
	void SetLastDamageType(ZDAMAGETYPE type) { MEMBER_SET_CHECKCRC(m_damageInfo, m_LastDamageType, type); }

	bool DoingStylishMotion();
	
	bool IsObserverTarget();

	MMatchTeam GetTeamID() { return m_nTeamID.Ref(); }
	void SetTeamID(MMatchTeam nTeamID) { m_nTeamID.Set_CheckCrc(nTeamID); }
	bool IsSameTeam(ZCharacter* pCharacter) 
	{ 
		if (pCharacter->GetTeamID() == -1) return false;
		if (pCharacter->GetTeamID() == GetTeamID()) return true; return false; 
	}
	bool IsTagger() { return m_dwStatusBitPackingValue.Ref().m_bTagger; }
	void SetTagger(bool bTagger) { m_dwStatusBitPackingValue.Ref().m_bTagger = bTagger; }

	void SetLastThrower(MUID uid, float fTime) { MEMBER_SET_CHECKCRC(m_damageInfo, m_LastThrower, uid); MEMBER_SET_CHECKCRC(m_damageInfo, m_tmLastThrowClear, fTime); }
	const MUID& GetLastThrower() { return m_damageInfo.Ref().m_LastThrower; }
	float GetLastThrowClearTime() { return m_damageInfo.Ref().m_tmLastThrowClear; }

	// 동작이나 이벤트에 관한 것들.
	//void Damaged(ZCharacter* pAttacker, rvector& dir, RMeshPartsType partstype,MMatchCharItemParts wtype,int nCount=-1);
	//void DamagedGrenade(MUID uidOwner, rvector& dir, float fDamage,int nTeamID);
	//void DamagedFalling(float fDamage);
	//void DamagedKatanaSplash(ZCharacter* pAttacker,float fDamageRange);

	void Revival();
	void Die();
	void ActDead();
	__forceinline void InitHPAP();	
	virtual void InitStatus();
	virtual void InitRound();
	virtual void InitItemBullet();

	void TestChangePartsAll();
	void TestToggleCharacter();

	virtual void OutputDebugString_CharacterState();

	void ToggleClothSimulation();
	void ChangeLowPolyModel();
	bool IsFallingToNarak() { return m_dwStatusBitPackingValue.Ref().m_bFallingToNarak; }

	MMatchItemDesc* GetSelectItemDesc() {
		if(GetItems())
			if(GetItems()->GetSelectedWeapon())
				return GetItems()->GetSelectedWeapon()->GetDesc();
		return NULL;
	}

	void LevelUp();
	void LevelDown();

	bool Save(ZFile *file);
	bool Load(ZFile *file,int nVersion);	// 나중에 MZFile * 로 포팅

	RMesh *GetWeaponMesh(MMatchCharItemParts parts);

	virtual float ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out=0);
	virtual bool IsAttackable();

	virtual bool IsGuard();
	virtual void OnMeleeGuardSuccess();


	virtual void OnDamagedAnimation(ZObject *pAttacker,int type);

	// ZObject에 맞게 만든 동작이나 이벤트에 관한 것들.
	virtual ZOBJECTHITTEST HitTest( const rvector& origin, const rvector& to, float fTime ,rvector *pOutPos=NULL );

	virtual void OnKnockback(rvector& dir, float fForce);
//	virtual void OnDamage(int damage, float fRatio = 1.0f);
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1);
	virtual void OnDamagedAPlayer(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeTpye=-1);
	virtual void OnDamagedAPlayer(ZObject* pAttacker, vector<MTD_ShotInfo*> vShots);
	virtual void OnScream();

	int GetDTLastWeekGrade() { return m_MInitialInfo.Ref().nDTLastWeekGrade; }
	const MTD_CharInfo* GetCharInfo() const { return &m_MInitialInfo.Ref(); }
};

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, const unsigned long int* pItemID);
void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, unsigned long int nWeaponID);
bool CheckTeenVersionMesh(RMesh** ppMesh);

//dll-injection으로 호출하지 못하도록 강제 인라이닝
__forceinline void ZCharacter::InitHPAP()
{	
	m_pModule_HPAP->SetMaxHP(GetMaxHP());
	m_pModule_HPAP->SetMaxAP(GetMaxAP());

	m_pModule_HPAP->SetHP(GetMaxHP());
	m_pModule_HPAP->SetAP(GetMaxAP());

	/*
	m_pModule_HPAP->SetMaxHP(m_Property.fMaxHP.Ref());
	m_pModule_HPAP->SetMaxAP(m_Property.fMaxAP.Ref());

	m_pModule_HPAP->SetHP(m_Property.fMaxHP.Ref());
	m_pModule_HPAP->SetAP(m_Property.fMaxAP.Ref());
	*/
}

//////////////////////////////////////////////////////////////////////////
//	ENUM
//////////////////////////////////////////////////////////////////////////
enum CHARACTER_LIGHT_TYPE
{
	GUN,
	SHOTGUN,
	CANNON,
	NUM_LIGHT_TYPE,
};

//////////////////////////////////////////////////////////////////////////
//	STRUCT
//////////////////////////////////////////////////////////////////////////
typedef struct
{
	int			iType;
	float		fLife;
	rvector		vLightColor;
	float		fRange;
}	sCharacterLight;




int gaepanEncode(int a, int depth);
int gaepanDecode(int a, int depth);

#endif