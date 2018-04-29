#include "stdafx.h"
#include "ReportView.h"
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
	m_hWnd = CreateWindow(WC_LISTVIEW, _T(""), LVS_REPORT | WS_CHILD | WS_VISIBLE,
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

	m_columns = columns;
	m_columnCount = columnCount;

	// Find string type column that will be located leftmost.
	m_leftMostColumnIndex = 0;
	for(int i = 0; i < columnCount; i++) {
		if(columns[i].type == Column::Type::String) {
			m_leftMostColumnIndex = i;
			break;
		}
	}

	LVCOLUMN col = { 0 };
	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	RECT rect;
	WIN32_ASSERT(GetClientRect(m_hWnd, &rect));
	auto width = rect.right - rect.left;
	auto remainingWidth = width;
	for(int i = 0; i < columnCount; i++) {
		col.iSubItem = col.iOrder = i;
		if(0 == i) {
			col.iOrder = m_leftMostColumnIndex;
		} else if(i <= m_leftMostColumnIndex) {
			col.iOrder = i - 1;
		}
		auto pCol = &columns[col.iOrder];
		switch(pCol->type) {
		case Column::Type::String:
			col.fmt = LVCFMT_LEFT;
			break;
		case Column::Type::Number:
			col.fmt = LVCFMT_RIGHT | LVCFMT_FIXED_WIDTH;
			break;
		}
		col.pszText = (LPTSTR)pCol->title;
		if(1 < pCol->width) {
			// Width specifies pixel length.
			col.cx = (int)pCol->width;
		} else if(0 < pCol->width) {
			// Width specifies percentage of width of List View.
			col.cx = (int)(width * pCol->width);
		} else if(0 == pCol->width) {
			// TODO: Implement automatic width setting.
			return E_NOTIMPL;
		} else {
			// Width is remaning length.
			col.cx = (0 < remainingWidth) ? remainingWidth : 0;
		}
		remainingWidth -= col.cx;
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

	return S_OK;
}

HRESULT CReportView::clear()
{
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);

	HR_ASSERT(ListView_DeleteAllItems(m_hWnd), E_UNEXPECTED);
	return S_OK;
}
