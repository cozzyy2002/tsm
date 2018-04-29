#pragma once

class CVar
{
public:
	CVar(LPCTSTR str) : str(str) {}
	CVar(int num, int radix = 10) {
		TCHAR str[40];
		_itot_s(num, str, radix);
		this->str = str;
	}
	LPCTSTR toString() const { return str.c_str(); }

protected:
	std::tstring str;
};

class CReportView
{
public:
	CReportView(HWND hWnd = NULL);
	~CReportView();

	HRESULT create(HINSTANCE hInst, HWND hWndParent, HWND* phWnd = nullptr);

	struct Column {
		enum class Type {
			String,
			Number,
		};
		Type type;
		LPCTSTR title;
		float width;
	};
	static const int autoColumnWidth = 0;
	static const int remainingColumnWidth = -1;

	template<int ColumnCount>
	HRESULT setColumns(const Column (&columns)[ColumnCount]);
	template<int ColumnCount>
	HRESULT setColumns(HWND hWnd, const Column(&columns)[ColumnCount]);
	HRESULT setColumns(HWND hWnd, const Column* columns, int columnCount);

	template<int ItemCount>
	HRESULT addItems(const CVar(&items)[ItemCount]);
	HRESULT addItems(const CVar* items, int itemCount);

	HRESULT clear();

protected:
	HWND m_hWnd;
	const Column* m_columns;
	int m_columnCount;
	int m_leftMostColumnIndex;
	static bool m_commonControlInitialized;
};

/**
 * Set column header and column attributes specified by columns argument.
 *
 * Before calling this method, call this->create() method to create List View control.
 */
template<int ColumnCount>
inline HRESULT CReportView::setColumns(const Column (&columns)[ColumnCount])
{
	return setColumns(NULL, &(columns[0]), ColumnCount);
}

/**
 * Set column header and column attributes specified by columns argument.
 *
 * Specify handle of List View control already crewated by hWnd argument.
 * This method is intended to set columns of List View contol created by dialog resource.
 * In this case, calling this->create() method is not necessary.
 */
template<int ColumnCount>
inline HRESULT CReportView::setColumns(HWND hWnd, const Column(&columns)[ColumnCount])
{
	return setColumns(hWnd, &(columns[0]), ColumnCount);
}

template<int ItemCount>
inline HRESULT CReportView::addItems(const CVar(&items)[ItemCount])
{
	return addItems(&items[0], ItemCount);
}
