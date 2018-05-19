#include "stdafx.h"
#include "ReportView.h"
#include "ClipBoard.h"
#include <StateMachine/Assert.h>

#include <CommCtrl.h>

#pragma comment(lib, "Comctl32.lib")

/*static*/ bool CReportView::m_commonControlInitialized = false;

CReportView::CReportView(HWND hWnd /*= NULL*/)
	: m_hWnd(hWnd), m_columns(nullptr), m_columnCount(0), m_leftMostColumnIndex(0)
{
}

CReportView::~CReportView()
{
}

/**
 * Creates ListView control with ReportView.
 *
 * Size of ListView control is set equal size of parent window.
 */
HRESULT CReportView::create(HINSTANCE hInst, HWND hWndParent, HWND* phWnd /*= nullptr*/)
{
	if(!m_commonControlInitialized) {
		m_commonControlInitialized = true;
		INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
		WIN32_ASSERT(InitCommonControlsEx(&icc));
	}

	RECT rect;
	WIN32_ASSERT(GetClientRect(hWndParent, &rect));
	m_hWnd = CreateWindow(WC_LISTVIEW, _T(""), LVS_REPORT | LVS_SHOWSELALWAYS | WS_VSCROLL | WS_CHILD | WS_VISIBLE,
		0, 0, rect.right - rect.left, rect.bottom - rect.top, hWndParent, (HMENU)NULL, hInst, NULL);
	WIN32_ASSERT(m_hWnd);

	if(phWnd) *phWnd = m_hWnd;
	return S_OK;
}

HRESULT CReportView::setColumns(HWND hWnd, const Column* columns, int columnCount)
{
	// m_hWnd should be set by create() or this method.
	if(hWnd) m_hWnd = hWnd;
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);
	ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_FULLROWSELECT);

	m_columns = columns;
	m_columnCount = columnCount;

	// Find string type column that will be located leftmost.
	// And calculate width of columns.
	m_leftMostColumnIndex = -1;
	RECT rect;
	WIN32_ASSERT(GetClientRect(m_hWnd, &rect));
	auto listViewWidth = rect.right - rect.left;
	auto remainingWidth = listViewWidth;
	auto remainingWidthCount = 0;
	std::unique_ptr<int[]> columnWidth(new int[columnCount]);
	static const int stringCharWidth = ListView_GetStringWidth(m_hWnd, _T("A"));
	static const int numberCharWidth = ListView_GetStringWidth(m_hWnd, _T("0"));
	for(int i = 0; i < columnCount; i++) {
		if((m_leftMostColumnIndex < 0) && (columns[i].type == Column::Type::String)) {
			m_leftMostColumnIndex = i;
		}
		auto pCol = &columns[i];
		auto& width = columnWidth[i];
		if(1 < pCol->width) {
			// Width specifies character length.
			width = (int)pCol->width * ((pCol->type == Column::Type::String) ? stringCharWidth : numberCharWidth);
		} else if(0 < pCol->width) {
			// Width specifies percentage of width of List View.
			width = (int)(listViewWidth * pCol->width);
		} else if(0 == pCol->width) {
			// autoColumnWidth = automatic width.
			// Sufficient width to show title in column header.
			width = _tcslen(pCol->title) * stringCharWidth;
		} else {
			// remainingColumnWidth = Width is remaning length.
			width = -1;		// Mark to set value later.
			remainingWidthCount++;
		}
		remainingWidth -= (0 < width) ? width : 0;
	}
	if(m_leftMostColumnIndex < 0) m_leftMostColumnIndex = 0;
	if(remainingWidthCount) {
		// Set width of the column(s) marked as remaining width.
		remainingWidth /= remainingWidthCount;
		for(int i = 0; i < columnCount; i++) {
			auto& width = columnWidth[i];
			if(width < 0) width = remainingWidth;
		}
	}

	LVCOLUMN col = { 0 };
	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	for(int i = 0; i < columnCount; i++) {
		col.iSubItem = col.iOrder = i;
		if(0 == i) {
			col.iOrder = m_leftMostColumnIndex;
		} else if(i <= m_leftMostColumnIndex) {
			col.iOrder = i - 1;
		}
		auto pCol = &columns[col.iOrder];
		col.cx = columnWidth[col.iOrder];
		switch(pCol->type) {
		case Column::Type::String:
			col.fmt = LVCFMT_LEFT;
			break;
		case Column::Type::Number:
			col.fmt = LVCFMT_RIGHT | LVCFMT_FIXED_WIDTH;
			break;
		case Column::Type::Bool:
			col.fmt = LVCFMT_CENTER | LVCFMT_FIXED_WIDTH;
			break;
		}
		col.pszText = (LPTSTR)pCol->title;
		WIN32_ASSERT(i == ListView_InsertColumn(m_hWnd, i, &col));
	}

	return S_OK;
}

HRESULT CReportView::addItems(const CVar* items, int itemCount, LPVOID itemData /*= nullptr*/)
{
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);
	HR_ASSERT(itemCount <= m_columnCount, E_INVALIDARG);

	auto iItem = ListView_GetItemCount(m_hWnd);
	for(int i = 0; i < itemCount; i++) {
		auto pItem = &items[i];
		if(0 == i) {
			// Insert leftmost item.
			if(m_leftMostColumnIndex < itemCount) {
				pItem = &items[m_leftMostColumnIndex];
			}
			LVITEM item = { LVIF_TEXT | LVIF_PARAM, iItem, i };
			item.pszText = (LPTSTR)pItem->toString();
			item.lParam = (LPARAM)itemData;
			HR_ASSERT(iItem == ListView_InsertItem(m_hWnd, &item), E_UNEXPECTED);
		} else {
			// Set remaining items as sub item.
			if(i <= m_leftMostColumnIndex) {
				pItem = &items[i - 1];
			}
			ListView_SetItemText(m_hWnd, iItem, i, (LPTSTR)pItem->toString());
		}
	}
	// Make item just added visible.
	ListView_EnsureVisible(m_hWnd, iItem, FALSE);

	return S_OK;
}

/**
 * Returns setlected index in the list view.
 *
 * If no item is selected, returns -1.
 */
int CReportView::getSelectedIndex(int iStart /*= -1*/) const
{
	return ListView_GetNextItem(m_hWnd, iStart, LVNI_SELECTED);
}

std::tstring CReportView::getItemText(int iItem, int iSubItem /*= 0*/) const
{
	TCHAR text[0x100] = _T("");
	LVITEM item = { LVIF_TEXT, iItem, iSubItem };
	if(iSubItem < m_leftMostColumnIndex) {
		item.iSubItem++;
	} else if(iSubItem == m_leftMostColumnIndex) {
		item.iSubItem = 0;
	}
	item.pszText = text;
	item.cchTextMax = ARRAYSIZE(text);
	HR_EXPECT(ListView_GetItem(m_hWnd, &item), E_UNEXPECTED);
	return text;
}

LPVOID CReportView::getItemData(int iItem) const
{
	LVITEM item = { LVIF_PARAM, iItem, 0 };
	item.lParam = (LPARAM)nullptr;
	HR_EXPECT(ListView_GetItem(m_hWnd, &item), E_UNEXPECTED);
	return LPVOID(item.lParam);
}

// Copies currently selected ListView items to clip board as CSV Unicode text.
// If no item is selected, copy all items.
HRESULT CReportView::copy()
{
	std::wstringstream stream;
	LPCWSTR comma = L"";
	LPCWSTR eol = L"\r\n";

	// Columns
	for(int i = 0; i < m_columnCount; i++) {
		CT2W wstr(m_columns[i].title);
		stream << comma << L"\"" << (LPCWSTR)wstr << L"\"";
		comma = L",";
	}
	stream << eol;

	// Items
	UINT flag = ListView_GetSelectedCount(m_hWnd) ? LVNI_SELECTED : LVNI_ALL;
	int iItem = -1;
	while(true) {
		iItem = ListView_GetNextItem(m_hWnd, iItem, flag);
		if(iItem < 0) break;

		comma = L"";
		for(int i = 0; i < m_columnCount; i++) {
			CT2W wstr(getItemText(iItem, i).c_str());
			stream << comma << L"\"" << (LPCWSTR)wstr << L"\"";
			comma = L",";
		}
		stream << eol;
	}

	// Copy text to clip board.
	CClipBoard cl;
	WIN32_ASSERT(cl.open(m_hWnd));
	auto str = stream.str();
	HR_ASSERT_OK(cl.copy(CF_UNICODETEXT, str.c_str(), (str.size() + 1) * sizeof(WCHAR)));

	return S_OK;
}

HRESULT CReportView::clear()
{
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);

	HR_ASSERT(ListView_DeleteAllItems(m_hWnd), E_UNEXPECTED);
	return S_OK;
}

void CReportView::resize(int width, int height)
{
	// Note: Prvent resizing until main windows appears.
	if(m_hWnd && ListView_GetItemCount(m_hWnd)) {
		WIN32_EXPECT(MoveWindow(m_hWnd, 0, 0, width, height, TRUE));
	}
}
