#ifndef MLISTBOX_H
#define MLISTBOX_H

#include "MWidget.h"
//#include "CMPtrList.h"
#include "CMList.h"
#include "MScrollBar.h"
#include "MColorTable.h"

enum MListViewStyle
{
	MVS_LIST = 0,
	MVS_ICON
};

class MListItem {
public:
	bool m_bSelected;

	MListItem(void) { m_bSelected = false; }
	virtual ~MListItem(void) {}

	virtual const char* GetString(void) = 0;
	virtual const char* GetString(int i) {
		if (i == 0) return GetString();
		return NULL;
	}
	virtual void SetString(const char* szText) {
	}

	MBitmap* GetBitmap(void) {
		return GetBitmap(0);
	}
	virtual MBitmap* GetBitmap(int i) {
		return NULL;
	}

	virtual const MCOLOR GetColor(void) { return GetColor(0); }
	virtual const MCOLOR GetColor(int i) {
		return MCOLOR(DEFCOLOR_MLIST_TEXT);
	}

	// 드래그 & 드롭을 했을때 전송될 스트링
	virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString) {
		return false;
	}
};

class MListFieldItem {
protected:
	MCOLOR		m_Color;
	char* m_szString;
	MBitmap* m_pBitmap;
public:
	MListFieldItem(const char* szString, MCOLOR color) {
		m_Color = color;
		m_szString = NULL;
		SetString(szString);
		m_pBitmap = NULL;
	}
	MListFieldItem(const char* szString) {
		m_Color = MCOLOR(DEFCOLOR_MLIST_TEXT);
		m_szString = NULL;
		SetString(szString);
		m_pBitmap = NULL;
	}
	MListFieldItem(MBitmap* pBitmap) {
		m_Color = MCOLOR(DEFCOLOR_MLIST_TEXT);
		m_szString = NULL;
		m_pBitmap = pBitmap;
	}
	virtual ~MListFieldItem(void) {
		if (m_szString != NULL) {
			delete[] m_szString;
			m_szString = NULL;
		}
		m_szString = NULL;
	}

	virtual const char* GetString(void) {
		return m_szString;
	}

	virtual void SetString(const char* szString) {
		if (m_szString != NULL) {
			delete[] m_szString;
		}

		// Allocate memory for the new string (+1 for null terminator)
		size_t stringLength = strlen(szString) + 1;
		m_szString = new char[stringLength];

		// Use the correct size for strcpy_s
		strcpy_s(m_szString, stringLength, szString);
	}


	void SetColor(MCOLOR color) { m_Color = color; }
	virtual const MCOLOR GetColor() { return m_Color; }

	MBitmap* GetBitmap(void) { return m_pBitmap; }
	void SetBitmap(MBitmap* pBitmap) { m_pBitmap = pBitmap; }
};


class MDefaultListItem : public MListItem {
	CMPtrList<MListFieldItem>	m_Items;
public:
	MDefaultListItem(void) {
	}
	MDefaultListItem(const char* szText, const MCOLOR color) {
		MListFieldItem* pNew = new MListFieldItem(szText, color);
		m_Items.Add(pNew);
	}
	MDefaultListItem(const char* szText) {
		MListFieldItem* pNew = new MListFieldItem(szText);
		m_Items.Add(pNew);
	}
	MDefaultListItem(MBitmap* pBitmap, const char* szText) {
		MListFieldItem* pNew = new MListFieldItem(pBitmap);
		m_Items.Add(pNew);
		pNew = new MListFieldItem(szText);
		m_Items.Add(pNew);
	}
	virtual ~MDefaultListItem(void) {
		for (int i = 0; i < m_Items.GetCount(); i++) {
			MListFieldItem* pItem = m_Items.Get(i);
			delete pItem;
		}
	}
	virtual const char* GetString(void) {
		if (m_Items.GetCount() > 0) return m_Items.Get(0)->GetString();
		return NULL;
	}
	virtual const char* GetString(int i) {
		if (i < m_Items.GetCount()) return m_Items.Get(i)->GetString();
		return NULL;
	}
	virtual void SetString(const char* szText) {
		if (m_Items.GetCount()) {
			delete m_Items.Get(0);
			m_Items.Delete(0);
		}
		MListFieldItem* pNew = new MListFieldItem(szText);
		m_Items.MoveFirst();
		m_Items.InsertBefore(pNew);
	}

	virtual MBitmap* GetBitmap(int i) {
		if (i < m_Items.GetCount()) return m_Items.Get(i)->GetBitmap();
		return NULL;
	}

	virtual const MCOLOR GetColor() {
		if (m_Items.GetCount() > 0) return m_Items.Get(0)->GetColor();
		return MCOLOR(DEFCOLOR_MLIST_TEXT);
	}
	/*
	virtual int GetBitmapMaxHeight(void){
		int nHeight = 0;
		for(int i=0; i<m_Items.GetCount(); i++){
			MListFieldItem* pItem = m_Items.Get(i);
			MBitmap* pBitmap = pItem->GetBitmap();
			if(pBitmap!=NULL) nHeight = max(nHeight, pBitmap->GetHeight());
		}
		return nHeight;
	}
	*/
};

struct MLISTFIELD {
	char			szFieldName[256];
	int				nTabSize;
};

class MListBox;
class MListBoxLook {
public:
	MCOLOR	m_SelectedPlaneColor;
	MCOLOR	m_SelectedTextColor;
	MCOLOR	m_UnfocusedSelectedPlaneColor;
	MAlignmentMode	m_ItemTextAlignmentMode;
	bool			m_bItemTextMultiLine;
	MBitmap* m_pItemSlotBitmap;
protected:
	virtual void OnHeaderDraw(MDrawContext* pDC, MRECT& r, const char* szText);
	virtual int OnItemDraw(MDrawContext* pDC, MRECT& r, const char* szText, MCOLOR color, bool bSelected, bool bFocus, int nAdjustWidth = 0);
	virtual int OnItemDraw(MDrawContext* pDC, MRECT& r, MBitmap* pBitmap, bool bSelected, bool bFocus, int nAdjustWidth = 0);
	virtual void OnFrameDraw(MListBox* pListBox, MDrawContext* pDC);
public:
	MListBoxLook(void);

	virtual void OnDraw(MListBox* pListBox, MDrawContext* pDC);
	virtual MRECT GetClientRect(MListBox* pListBox, MRECT& r);
};

typedef void (*ZCB_ONDROP)(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

class MListBox : public MWidget {
protected:
	class SortedItemList : public CMLinkedList<MListItem> {
	public:
		bool	m_bAscend;
	public:
		SortedItemList(void) {
			m_bAscend = true;
		}
		virtual int Compare(MListItem* lpRecord1, MListItem* lpRecord2) {
			int nCompare = _stricmp(lpRecord1->GetString(0), lpRecord2->GetString(0));
			if (m_bAscend == true) return nCompare;
			else return -nCompare;
		}
	} m_Items;
	int				m_nOverItem;
	int				m_nSelItem;
	int				m_nShowItemCount;
	int				m_nStartItemPos;
	int				m_nItemHeight;
	MScrollBar* m_pScrollBar;

	CMLinkedList<MLISTFIELD>	m_Fields;

	bool			m_bVisibleHeader;
	bool			m_bSelected;
	MListViewStyle	m_ViewStyle;
	bool			m_bAlwaysVisibleScrollbar;
	bool			m_bDragAndDrop;
	ZCB_ONDROP		m_pOnDropFunc;

public:
	bool			m_bAbsoulteTabSpacing;
	bool			m_bHideScrollBar;
	bool			m_bNullFrame;
	bool			m_bMultiSelect;

protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
	virtual bool IsDropable(MWidget* pSender) { return m_bDragAndDrop; }

	void RecalcList(void);
	void RecalcScrollBar(void);

	int FindNextItem(int i, char c);

	virtual void OnSize(int w, int h);

	virtual bool OnCommand(MWidget* pWindow, const char* szMessage);

	void Initialize(void);

public:
	MCOLOR m_FontColor;
	MAlignmentMode m_FontAlign;

public:
	MListBox(const char* szName, MWidget* pParent = NULL, MListener* pListener = NULL);
	MListBox(MWidget* pParent = NULL, MListener* pListener = NULL);
	virtual ~MListBox(void);

	void Add(const char* szItem);
	void Add(const char* szItem, MCOLOR color);
	void Add(MListItem* pItem);
	const char* GetString(int i);
	MListItem* Get(int i);
	bool Set(int i, const char* szItem);
	bool Set(int i, MListItem* pItem);
	void Remove(int i);
	void RemoveAll(void);
	bool Swap(int i, int j);
	int GetCount(void);
	int GetSelIndex(void);
	bool SetSelIndex(int i);
	const char* GetSelItemString(void);
	MListItem* GetSelItem(void);

	// multiselect 관련
	int GetSelCount(void);

	int FindItem(MPOINT& p);
	bool GetItemPos(MPOINT* p, int i);

	int GetItemHeight(void);
	void SetItemHeight(int nHeight);

	bool IsShowItem(int i);
	void ShowItem(int i);
	void SetStartItem(int i);
	int GetStartItem(void);
	int GetShowItemCount(void);

	MScrollBar* GetScrollBar(void);

	void Sort(bool bAscend = true);

	// Field Support
	void AddField(const char* szFieldName, int nTabSize);
	void RemoveField(const char* szFieldName);
	MLISTFIELD* GetField(int i);
	int GetFieldCount(void);
	void RemoveAllField(void);

	bool IsVisibleHeader(void);
	void SetVisibleHeader(bool bVisible);

	bool IsAlwaysVisibleScrollbar(void);
	void SetAlwaysVisibleScrollbar(bool bVisible);

	bool IsSelected() { return m_bSelected; }
	void SetSelected(bool bSelected) { m_bSelected = bSelected; }
	MListViewStyle GetViewStyle() { return m_ViewStyle; }
	void SetViewStyle(MListViewStyle ViewStyle);
	int GetTabSize();
	void EnableDragAndDrop(bool bEnable);

	void SetOnDropCallback(ZCB_ONDROP pCallback) { m_pOnDropFunc = pCallback; }

	virtual void MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight);


	DECLARE_LOOK(MListBoxLook)
	DECLARE_LOOK_CLIENT()

#define MINT_LISTBOX	"ListBox"
	virtual const char* GetClassName(void) { return MINT_LISTBOX; }
};

#define MLB_ITEM_SEL		"selected"
#define MLB_ITEM_SEL2		"selected2"
#define MLB_ITEM_DBLCLK		"dclk"
#define MLB_ITEM_SELLOST	"lost"
#define MLB_ITEM_DEL		"del"
#define MLB_ITEM_START		"start"			
#define MLB_ITEM_CLICKOUT	"clickout"

#endif
