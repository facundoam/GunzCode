#include "stdafx.h"
#include "MBButtonLook.h"
#include "MColorTable.h"
#include "MBitmapDrawer.h"
#include "Mint.h"
#include "cmath"

MBButtonLook::MBButtonLook(void) : MScalableLook()
{
	int i;
	for (i = 0; i < 9; i++) {
		m_pUpBitmaps[i] = NULL;
		m_pDownBitmaps[i] = NULL;
		m_pOverBitmaps[i] = NULL;
	}
	for (i = 0; i < 4; i++) {
		m_pFocusBitmaps[i] = NULL;
	}

	m_pFont = NULL;
	m_FontColor = MCOLOR(DEFCOLOR_MBUTTON_TEXT);
	m_FontDownColor = MCOLOR(DEFCOLOR_LIGHT);
	m_FontDisableColor = MCOLOR(DEFCOLOR_DISABLE);
	m_FontHighlightColor = MCOLOR(DEFCOLOR_PEN_HIGHLIGHT);
	m_bStretch = true;
	m_bCustomLook = false;

	m_FontDownOffset = MPOINT(0, 0);
}

void MBButtonLook::DrawText(MButton* pButton, MDrawContext* pDC, MRECT& r, const char* szText, MAlignmentMode a)
{
	if (pButton->m_pIcon != NULL) {
		pDC->SetBitmap(pButton->m_pIcon);
		pDC->Draw(r.x, r.y + (r.h - pButton->m_pIcon->GetHeight()) / 2);

		r.x += pButton->m_pIcon->GetWidth();
		r.w -= pButton->m_pIcon->GetWidth();
	}

	if (pButton->IsButtonDown())
	{
		r.x += (int)(m_FontDownOffset.x * GetScale());
		r.y += (int)(m_FontDownOffset.y * GetScale());
	}

	if (m_pFont != NULL) pDC->SetFont(m_pFont);
	if (pButton->IsEnable() == true) {
		if (pButton->IsButtonDown() == true) pDC->SetColor(m_FontDownColor);
		else if (pButton->IsMouseOver()) pDC->SetColor(m_FontDownColor);
		else if (pButton->GetType() == MBT_PUSH2 && pButton->GetCheck()) pDC->SetColor(m_FontDownColor);
		else pDC->SetColor(m_FontColor);
	}
	else pDC->SetColor(m_FontDisableColor);
	if (pButton->m_bHighlight == true) {
		MCOLOR PrevHCol = pDC->SetHighlightColor(m_FontHighlightColor);
		pDC->TextWithHighlight(r, szText, a);
		pDC->SetHighlightColor(PrevHCol);
	}
	else {
		pDC->Text(r, szText, a);
	}
}

void MBButtonLook::DrawFocus(MDrawContext* pDC, MRECT& r)
{
	if (m_pFocusBitmaps[0] == NULL) return;

	MRECT cr = pDC->GetClipRect();
	MRECT pcr = cr;
	cr.x -= 2;
	cr.y -= 2;
	cr.w += 4;
	cr.h += 4;
	pDC->SetClipRect(cr);

	pDC->SetBitmap(m_pFocusBitmaps[0]);
	pDC->Draw(r.x - 2, r.y - 2);
	pDC->SetBitmap(m_pFocusBitmaps[1]);
	pDC->Draw(r.x + r.w - m_pFocusBitmaps[1]->GetWidth() + 2, r.y - 2);
	pDC->SetBitmap(m_pFocusBitmaps[2]);
	pDC->Draw(r.x - 2, r.y + r.h - m_pFocusBitmaps[1]->GetHeight() + 2);
	pDC->SetBitmap(m_pFocusBitmaps[3]);
	pDC->Draw(r.x + r.w - m_pFocusBitmaps[1]->GetWidth() + 2, r.y + r.h - m_pFocusBitmaps[1]->GetHeight() + 2);

	pDC->SetClipRect(pcr);
}

void MBButtonLook::OnDownDraw(MButton* pButton, MDrawContext* pDC)
{
	MRECT r = pButton->GetInitialClientRect();
	if (GetCustomLook())
	{
		DrawBitmapButtonCustom1(pDC, r, m_pDownBitmaps, true, m_bStretch);
		r = pButton->GetClientRect();
		r.x += 1;
		r.y += 1;
		r.w += 1;
		r.h += 1;
	}
	else
	{
		DrawBitmapFrame9(pDC, r, m_pDownBitmaps, m_bStretch, GetScale());
		r = pButton->GetClientRect();
	}
	if (pButton->IsFocus() == true) DrawFocus(pDC, r);

	DrawText(pButton, pDC, r, pButton->m_szName, pButton->GetAlignment());
}

void MBButtonLook::OnUpDraw(MButton* pButton, MDrawContext* pDC)
{
	if (pButton->GetType() == MBT_PUSH2 && pButton->GetCheck())
	{
		OnDownDraw(pButton, pDC);
		return;
	}

	MRECT r = pButton->GetInitialClientRect();
	if (GetCustomLook())
		DrawBitmapButtonCustom1(pDC, r, m_pUpBitmaps, false, m_bStretch);
	else
		DrawBitmapFrame9(pDC, r, m_pUpBitmaps, m_bStretch, GetScale());
	if (pButton->IsFocus() == true) DrawFocus(pDC, r);

	DrawText(pButton, pDC, pButton->GetClientRect(), pButton->m_szName, pButton->GetAlignment());
}

void MBButtonLook::OnOverDraw(MButton* pButton, MDrawContext* pDC)
{
	if (pButton->GetType() == MBT_PUSH2)
	{
		if (pButton->GetCheck()) OnDownDraw(pButton, pDC);
		else OnUpDraw(pButton, pDC);
		return;
	}

	MRECT r = pButton->GetInitialClientRect();
	if (GetCustomLook())
		DrawBitmapButtonCustom1(pDC, r, m_pOverBitmaps, false, m_bStretch);
	else
		DrawBitmapFrame9(pDC, r, m_pOverBitmaps, m_bStretch, GetScale());
	if (pButton->IsFocus() == true) DrawFocus(pDC, r);

	DrawText(pButton, pDC, pButton->GetClientRect(), pButton->m_szName, pButton->GetAlignment());
}

void MBButtonLook::OnDisableDraw(MButton* pButton, MDrawContext* pDC)
{
	OnUpDraw(pButton, pDC);
}

void MBButtonLook::OnDraw(MButton* pButton, MDrawContext* pDC)
{
	if (GetWireLook())		// ������ �����ʿ� �ٴ� ����� ��쿡...
	{
		MRECT rect = pButton->GetInitialClientRect();

		// ��� �׸���...
		pDC->SetColor(19, 19, 19, 255);
		pDC->FillRectangle(rect);


		// ��ư�� �׷��� ũ�⸦ ���Ѵ�.
		MRECT rectButtonBmp;
		rectButtonBmp.x = rect.x + rect.w - rect.h + 1;		// ��ư�� �׷��� ���� ��ġ�� ���Ѵ�.
		rectButtonBmp.y = rect.y + 1;						// �׵θ� �β�(1 pixel)������ 1 pixel �Ʒ��� ������ �׸���.
		rectButtonBmp.w = rect.h - 2;						// ������ ���̸� �������� ���� ���̸� �����.
		rectButtonBmp.h = rect.h - 2;						// ������ ���̸� �������� ���� ���̸� �����.
		float fScale = (float)(rect.h - 2) / (float)m_pDownBitmaps[1]->GetHeight();

		// ��ư�� ��Ʈ���� �׸���
		if (pButton->GetComboDropped())
		{
			HLineBitmap(pDC, rectButtonBmp.x, rectButtonBmp.y, rectButtonBmp.w, m_pDownBitmaps[2], false, fScale);
		}
		else
		{
			if (pButton->IsFocus())
				HLineBitmap(pDC, rectButtonBmp.x, rectButtonBmp.y, rectButtonBmp.w, m_pDownBitmaps[0], false, fScale);
			else
				HLineBitmap(pDC, rectButtonBmp.x, rectButtonBmp.y, rectButtonBmp.w, m_pDownBitmaps[1], false, fScale);
		}

		MRECT rectText = rect;
		rectText.w -= rect.h;
		pDC->SetColor(205, 205, 205, 255);
		pDC->Text(rectText, pButton->m_szName, pButton->GetAlignment());

		// �׵θ� �׸���...
		pDC->SetColor(205, 205, 205, 255);
		pDC->Rectangle(rect);

	}
	else
		MButtonLook::OnDraw(pButton, pDC);
}

#define CHECKBOX_SIZE  12	//800*600 �������� �� ũ��
void MBButtonLook::OnCheckBoxDraw(MButton* pButton, MDrawContext* pDC, bool bPushed)
{
	// Calculate the checkbox size based on workspace dimensions, rounding to the nearest integer
	const int boxWidth = static_cast<int>(std::round(CHECKBOX_SIZE * MGetWorkspaceWidth() / 800.f));
	const int boxHeight = static_cast<int>(std::round(CHECKBOX_SIZE * MGetWorkspaceHeight() / 600.f));

	// Get the button's initial client rectangle and calculate the position
	MRECT r = pButton->GetInitialClientRect();
	int x = r.x + boxWidth;  // Adjust x position based on box width
	int y = r.y + static_cast<int>(std::round(r.h * 0.5)) - static_cast<int>(std::round(boxHeight * 0.5));

	// Draw the checkbox
	pDC->SetColor(128, 128, 128, 255);
	pDC->Rectangle(x, y, boxWidth, boxHeight);

	// Draw the checkbox icon if pushed
	if (bPushed)
	{
		pDC->SetBitmap(m_pUpBitmaps[0]);
		pDC->Draw(x - static_cast<int>(std::round(boxWidth * 0.5)),
			y - static_cast<int>(std::round(boxHeight * 0.5)),
			static_cast<int>(std::round(boxWidth * 1.8f)),
			static_cast<int>(std::round(boxHeight * 1.8f)));
	}

	// Draw the button text
	r = pButton->GetClientRect();
	r.x += boxWidth;  // Adjust x position for the text
	DrawText(pButton, pDC, r, pButton->m_szName, pButton->GetAlignment());
}

MRECT MBButtonLook::GetClientRect(MButton* pButton, MRECT& r)
{
	float fScale = GetScale();

	int al = (int)(fScale * GETWIDTH(m_pUpBitmaps[3]));
	int au = (int)(fScale * GETHEIGHT(m_pUpBitmaps[7]));
	int ar = (int)(fScale * GETWIDTH(m_pUpBitmaps[5]));
	int ab = (int)(fScale * GETHEIGHT(m_pUpBitmaps[1]));
	return MRECT(r.x + al, r.y + au, r.w - (al + ar), r.h - (au + ab));
}

