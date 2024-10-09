#ifndef _MBASELOCALE_H
#define _MBASELOCALE_H

enum MCountry
{
	MC_INVALID = 0,
	MC_KOREA = 82,
	MC_US = 1,
	MC_JAPAN = 81,
	MC_BRAZIL = 55,
	MC_INDIA = 91,
	MC_NHNUSA = 10001
};

enum MLanguage
{
	ML_INVALID = 0x00,
	ML_CHINESE = LANG_CHINESE,
	ML_CHINESE_TRADITIONAL = SUBLANG_CHINESE_TRADITIONAL,
	ML_KOREAN = LANG_KOREAN,
	ML_ENGLISH = LANG_ENGLISH,
	ML_JAPANESE = LANG_JAPANESE,
	ML_BRAZIL = LANG_PORTUGUESE,
	ML_INDIA = LANG_INDONESIAN,
	ML_GERMAN = LANG_GERMAN,
	ML_SPANISH = LANG_SPANISH,
};

class MBaseLocale
{
private:
	void InitLanguageFromCountry();
protected:
	MCountry			m_nCountry;
	MLanguage			m_nLanguage;

	bool				m_bIsComplete;

	virtual bool OnInit() = 0;
public:
	MBaseLocale();
	virtual ~MBaseLocale();
	bool Init(MCountry nCountry);

	const MCountry	GetCountry() const { return m_nCountry; }
	const MLanguage GetLanguage() const { return m_nLanguage; }
	void SetLanguage(MLanguage langID) { m_nLanguage = langID; }	// 국가에 따라 초기화된 언어를 다른 것으로 바꾸고 싶을때

	const bool bIsComplete() const { return m_bIsComplete; }
};

const MCountry GetCountryID(const char* pCountry);
const MLanguage GetLanguageID(const char* pLanguage);

#endif