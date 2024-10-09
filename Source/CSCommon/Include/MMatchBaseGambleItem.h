#ifndef _MMATCHBASEGAMBLEITEM_H
#define _MMATCHBASEGAMBLEITEM_H

class MMatchBaseGambleItem
{
protected:
    MUID    m_uidItem;
    DWORD   m_dwGambleItemID;
    int     m_nItemCount;

protected:
    // Default constructor
    MMatchBaseGambleItem()
        : m_uidItem(), m_dwGambleItemID(0), m_nItemCount(0) {}

public:
    // Parameterized constructor using initializer list
    MMatchBaseGambleItem(const MUID& uidItem, DWORD dwGambleItemID, int nItemCount = 1)
        : m_uidItem(uidItem), m_dwGambleItemID(dwGambleItemID), m_nItemCount(nItemCount) {}

    virtual ~MMatchBaseGambleItem() {}

    // Getter methods
    const MUID& GetUID() const { return m_uidItem; }
    DWORD GetGambleItemID() const { return m_dwGambleItemID; }
    int GetItemCount() const { return m_nItemCount; }

    // Setter for item count
    void SetItemCount(int nVal) { m_nItemCount = nVal; }
};

#endif