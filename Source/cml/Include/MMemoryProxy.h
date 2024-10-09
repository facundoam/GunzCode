#pragma once

#include "MMath.h"
#include "MCRC32.h"

class MCrc32Container
{
	std::map<void*, DWORD> m_mapCrc32;
	typedef std::map<void*, DWORD>::iterator Iterator;

public:
	void Add(void* key, DWORD crc)
	{
		m_mapCrc32[key] = crc;
	}
	void Remove(void* key)
	{
		m_mapCrc32.erase(key);
	}
	DWORD Get(void* key)
	{
		Iterator it = m_mapCrc32.find(key);
		if (it!=m_mapCrc32.end())
			return it->second;
		return 0;
	}
};

extern void (*g_fpOnCrcFail)();	// crc체크 실패시(해킹시) 호출할 함수 포인터

MCrc32Container* GetCrcContainer();


#define PTR_OFFSET 0x66D6
// 참고로 한국 핵유저들이 보통 검색해보는 힙 범위는 0x15000000 ~ 0x30000000 정도
// (↑RandMaskVal.exe에 의해서 빌드 전에 변경되는 값. define 명을 수정하면 파싱 에러가 납니다.)

template <typename T>
class MProtectValue
{
	// 값을 할당한 포인터 주소값에도 마스킹값을 얹어 주소자체를 숨긴다
	typedef unsigned char* MaskedPtr;
	MaskedPtr m_pValue;

private:
	MaskedPtr	ToMaskedPtr(T* ptr) const			{ return MaskedPtr(ptr) + (PtrToUlong(this)+PTR_OFFSET); }
	T*			ToNormalPtr(MaskedPtr mptr) const	{ return (T*)(mptr - (PtrToUlong(this)+PTR_OFFSET)); }

	DWORD BuildCrc()
	{
		BYTE* pData = (BYTE*)ToNormalPtr(m_pValue);
		return MCRC32::BuildCRC32(pData, sizeof(T));
	}

public:
	MProtectValue()		 { m_pValue = ToMaskedPtr(new T); }

	~MProtectValue()	 { 
		delete ToNormalPtr(m_pValue);
		::GetCrcContainer()->Remove(this);
	}

	// 값을 레퍼런스로 얻는다
			T& Ref()		{ return *ToNormalPtr(m_pValue); }
	const	T& Ref() const	{ return *ToNormalPtr(m_pValue); }

	// 값 배정
	void Set(const T& newVal)		{ Ref() = newVal; }

	// 값 배정하면서 crc검사
	bool Set_CheckCrc(const T& newVal) {
		CheckCrc();
		Set(newVal);
		MakeCrc();
		return true;
	}
	// 값 배정하면서 crc 생성 (최초 값 배정용)
	void Set_MakeCrc(const T& newVal) {
		Set(newVal);
		MakeCrc();
	}

	// Set_CheckCrc()와 같다. 다만 T가 구조체일 경우 crc를 체크하며 멤버를 갱신하기 쉽게 하는 매크로 함수
#define MEMBER_SET_CHECKCRC(PROTECTVALUE, MEMBER, NEWVAL) { \
		PROTECTVALUE.CheckCrc(); \
		PROTECTVALUE.Ref().MEMBER = (NEWVAL); \
		PROTECTVALUE.MakeCrc(); \
	}

	// 재할당->복사를 통해 힙위치를 변경한다
	void ShiftHeapPos()
	{
		T* p = ToNormalPtr(m_pValue);
		m_pValue = ToMaskedPtr(new T);
		Ref() = *p;
		delete p;
	}

	void ShiftHeapPos_CheckCrc() { CheckCrc(); ShiftHeapPos(); }

	void MakeCrc()
	{
		DWORD crc = BuildCrc();
		::GetCrcContainer()->Add(this, crc);
	}

	// crc를 검사한다
	void CheckCrc()
	{
		if (BuildCrc() != ::GetCrcContainer()->Get(this))
			g_fpOnCrcFail();
	}

private:
	// 연산자 오버로드를 쓰면 편하겠지만 실제로 무슨 일이 일어나는지 명시적으로 표현하도록 강제하는 편이 낫다고 생각함.

	MProtectValue(T val) { m_pValue = ToMaskedPtr(new T(val)); }	// 복사생성자..혼란스럽다 차라리 숨기자
	MProtectValue<T>& operator=(int) {}	// 복사대입 연산자를 숨김
};