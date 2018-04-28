#include "stdafx.h"
#include "ReportView.h"
#include <StateMachine/Assert.h>

#include <CommCtrl.h>

#pragma comment(lib, "Comctl32.lib")

/*static*/ bool CReportView::m_commonControlInitialized = false;

CReportView::CReportView(HWND hWnd /*= NULL*/)
	: m_hWnd(hWnd), m_columns(nullptr)
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

HRESULT CReportView::setColumns(HWND hWnd, const Column * columns, int columnCount)
{
	// m_hWnd should be set by create() or this method.
	if(hWnd) m_hWnd = hWnd;
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);

	m_columns = columns;
	m_columnCount = columnCount;

	LVCOLUMN col = { 0 };
	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	RECT rect;
	WIN32_ASSERT(GetClientRect(m_hWnd, &rect));
	auto width = rect.right - rect.left;
	auto remainingWidth = width;
	for(int iCol = 0; iCol < columnCount; iCol++) {
		auto& column = columns[iCol];
		switch(column.type) {
		case Column::Type::String:
			col.fmt = LVCFMT_LEFT;
			break;
		case Column::Type::Number:
			col.fmt = LVCFMT_RIGHT | LVCFMT_FIXED_WIDTH;
			break;
		}
		col.iSubItem = iCol;
		col.pszText = (LPTSTR)column.title;
		if(1 < column.width) {
			// Width specifies pixel length.
			col.cx = (int)column.width;
		} else if(0 < column.width) {
			// Width specifies percentage of width of List View.
			col.cx = (int)(width * column.width);
		} else if(0 == column.width) {
			// TODO: Implement automatic width setting.
			return E_NOTIMPL;
		} else {
			// Width is remaning length.
			col.cx = (0 < remainingWidth) ? remainingWidth : 0;
		}
		remainingWidth -= col.cx;
		WIN32_ASSERT(iCol == ListView_InsertColumn(m_hWnd, iCol, &col));
	}

	return S_OK;
}

HRESULT CReportView::addItems(const CVar * items, int itemCount)
{
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);
	HR_ASSERT(itemCount <= m_columnCount, E_INVALIDARG);

	auto iItem = ListView_GetItemCount(m_hWnd);
	LVITEM item = { LVIF_TEXT, iItem, 0 };
	item.pszText = (LPTSTR)items->toString();
	HR_ASSERT(iItem == ListView_InsertItem(m_hWnd, &item), E_UNEXPECTED);
	for(int iCol = 1; iCol < itemCount; iCol++) {
		item.iSubItem++;
		item.pszText = (LPTSTR)(++items)->toString();
		HR_ASSERT(ListView_SetItem(m_hWnd, &item), E_UNEXPECTED);
	}

	return S_OK;
}

HRESULT CReportView::clear()
{
	HR_ASSERT(m_hWnd, E_ILLEGAL_METHOD_CALL);

	HR_ASSERT(ListView_DeleteAllItems(m_hWnd), E_UNEXPECTED);
	return S_OK;
}
