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
	for(int iCol = 0; iCol < columnCount; iCol++) {
		auto& column = columns[iCol];
		switch(column.type) {
		case Column::Type::String:
			col.fmt = LVCFMT_LEFT;
			break;
		case Column::Type::Number:
			col.fmt = LVCFMT_RIGHT;
			break;
		}
		col.iSubItem = iCol;
		col.pszText = (LPTSTR)column.title;
		col.cx = (int)column.width;
		WIN32_ASSERT(iCol <= ListView_InsertColumn(m_hWnd, 0, &col));
	}

	return S_OK;
}

HRESULT CReportView::setItem(int iCol, LPCTSTR str) const
{
	HR_ASSERT(iCol < m_columnCount, E_INVALIDARG);
	return S_OK;
}

HRESULT CReportView::setItem(int iCol, int num, int radix /*= 10*/) const
{
	TCHAR str[40];
	_itot_s(num, str, radix);
	return setItem(iCol, str);
}
