#include "stdafx.h"
#include "ReportView.h"
#include "ClipBoardUtil.h"
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
	ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_FULLROWSELECT);

	if(phWnd) *phWnd = m_hWnd;
	return S_OK;
}

HRESULT CReportView::setColumns(HWND hWnd, const Column* columns, int columnCount)
{
	// m_hWnd should be set by create() or this method.
	if(hWnd) m_hWnd = hWnd;
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);

	m_columns = columns;
	m_columnCount = columnCount;

	// Find string type column that will be located leftmost.
	// And calculate width of columns.
	m_leftMostColumnIndex = -1;
	RECT rect;
	WIN32_ASSERT(GetClientRect(m_hWnd, &rect));
	auto width = rect.right - rect.left;
	auto remainingWidth = width;
	std::unique_ptr<int[]> columnWidth(new int[columnCount]);
	for(int i = 0; i < columnCount; i++) {
		if((m_leftMostColumnIndex < 0) && (columns[i].type == Column::Type::String)) {
			m_leftMostColumnIndex = i;
		}
		auto pCol = &columns[i];
		auto& width = columnWidth[i];
		if(1 < pCol->width) {
			// Width specifies pixel length.
			width = (int)pCol->width;
		} else if(0 < pCol->width) {
			// Width specifies percentage of width of List View.
			width = (int)(width * pCol->width);
		} else if(0 == pCol->width) {
			// autoColumnWidth = automatic width.
			return E_NOTIMPL;
		} else {
			// remainingColumnWidth = Width is remaning length.
			width = (0 < remainingWidth) ? remainingWidth : 0;
		}
		remainingWidth -= width;
	}
	if(m_leftMostColumnIndex < 0) m_leftMostColumnIndex = 0;

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
		}
		col.pszText = (LPTSTR)pCol->title;
		WIN32_ASSERT(i == ListView_InsertColumn(m_hWnd, i, &col));
	}

	return S_OK;
}

HRESULT CReportView::addItems(const CVar* items, int itemCount)
{
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);
	HR_ASSERT(itemCount <= m_columnCount, E_INVALIDARG);

	auto iItem = ListView_GetItemCount(m_hWnd);
	for(int i = 0; i < itemCount; i++) {
		auto pItem = &items[i];
		LVITEM item = { LVIF_TEXT, iItem, i };
		if(0 == i) {
			// Insert leftmost item.
			if(m_leftMostColumnIndex < itemCount) {
				pItem = &items[m_leftMostColumnIndex];
			}
			item.pszText = (LPTSTR)pItem->toString();
			HR_ASSERT(iItem == ListView_InsertItem(m_hWnd, &item), E_UNEXPECTED);
		} else {
			// Set remaining items as sub item.
			if(i <= m_leftMostColumnIndex) {
				pItem = &items[i - 1];
			}
			item.pszText = (LPTSTR)pItem->toString();
			HR_ASSERT(ListView_SetItem(m_hWnd, &item), E_UNEXPECTED);
		}
	}
	// Make item just added visible.
	ListView_EnsureVisible(m_hWnd, iItem, FALSE);

	return S_OK;
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
		for(int iCol = 0; iCol < m_columnCount; iCol++) {
			TCHAR text[0x100];
			LVITEM item = { LVIF_TEXT, iItem, iCol };
			item.pszText = text;
			item.cchTextMax = ARRAYSIZE(text);
			WIN32_ASSERT(ListView_GetItem(m_hWnd, &item));
			CT2W wstr(text);
			stream << comma << L"\"" << (LPCWSTR)wstr << L"\"";
			comma = L",";
		}
		stream << eol;
	}

	// Copy text to clip board.
	CSafeClipBoard cl(m_hWnd);
	WIN32_ASSERT(cl.open());
	auto str = stream.str();
	CClipBoardBuffer buff;
	HR_ASSERT_OK(buff.copy(str.c_str(), (str.size() + 1) * sizeof(WCHAR)));
	WIN32_ASSERT(SetClipboardData(CF_UNICODETEXT, buff.get()));
	buff.detach();

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
