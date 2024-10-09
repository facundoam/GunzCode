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

extern void (*g_fpOnCrcFail)();	// crcüũ ���н�(��ŷ��) ȣ���� �Լ� ������

MCrc32Container* GetCrcContainer();


#define PTR_OFFSET 0x66D6
// ����� �ѱ� ���������� ���� �˻��غ��� �� ������ 0x15000000 ~ 0x30000000 ����
// (��RandMaskVal.exe�� ���ؼ� ���� ���� ����Ǵ� ��. define ���� �����ϸ� �Ľ� ������ ���ϴ�.)

template <typename T>
class MProtectValue
{
	// ���� �Ҵ��� ������ �ּҰ����� ����ŷ���� ��� �ּ���ü�� �����
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

	// ���� ���۷����� ��´�
			T& Ref()		{ return *ToNormalPtr(m_pValue); }
	const	T& Ref() const	{ return *ToNormalPtr(m_pValue); }

	// �� ����
	void Set(const T& newVal)		{ Ref() = newVal; }

	// �� �����ϸ鼭 crc�˻�
	bool Set_CheckCrc(const T& newVal) {
		CheckCrc();
		Set(newVal);
		MakeCrc();
		return true;
	}
	// �� �����ϸ鼭 crc ���� (���� �� ������)
	void Set_MakeCrc(const T& newVal) {
		Set(newVal);
		MakeCrc();
	}

	// Set_CheckCrc()�� ����. �ٸ� T�� ����ü�� ��� crc�� üũ�ϸ� ����� �����ϱ� ���� �ϴ� ��ũ�� �Լ�
#define MEMBER_SET_CHECKCRC(PROTECTVALUE, MEMBER, NEWVAL) { \
		PROTECTVALUE.CheckCrc(); \
		PROTECTVALUE.Ref().MEMBER = (NEWVAL); \
		PROTECTVALUE.MakeCrc(); \
	}

	// ���Ҵ�->���縦 ���� ����ġ�� �����Ѵ�
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

	// crc�� �˻��Ѵ�
	void CheckCrc()
	{
		if (BuildCrc() != ::GetCrcContainer()->Get(this))
			g_fpOnCrcFail();
	}

private:
	// ������ �����ε带 ���� ���ϰ����� ������ ���� ���� �Ͼ���� ��������� ǥ���ϵ��� �����ϴ� ���� ���ٰ� ������.

	MProtectValue(T val) { m_pValue = ToMaskedPtr(new T(val)); }	// ���������..ȥ�������� ���� ������
	MProtectValue<T>& operator=(int) {}	// ������� �����ڸ� ����
};