#pragma once

#include <vector>

class CVar
{
public:
	CVar(LPCTSTR str) : str(str) {}
	CVar(const std::tstring& str) : str(str) {}
	CVar(int num, int radix = 10) {
		TCHAR str[40];
		_itot_s(num, str, radix);
		this->str = str;
	}
	CVar(float num, LPCTSTR fmt = _T("%.3f")) {
		TCHAR str[40];
		_stprintf_s(str, fmt, num);
		this->str = str;
	}
	CVar(bool var) : str(var ? _T("x") : _T("")) {}
	CVar(void* p) {
		TCHAR str[(sizeof(p) * 2) + 5];
		_stprintf_s(str, _T("0x%p"), p);
		this->str = str;
	}

	LPCTSTR toString() const { return str.c_str(); }

protected:
	std::tstring str;
};

class CReportView
{
public:
	CReportView();
	~CReportView();

	struct Column {
		enum class Type {
			String,
			Number,
			Bool,
		};
		Type type;
		LPCTSTR title;
		float width;
	};
	static const int autoColumnWidth = 0;
	static const int remainingColumnWidth = -1;

	template<int ColumnCount>
	HRESULT setColumns(HWND hWnd, const Column(&columns)[ColumnCount]);
	HRESULT setColumns(HWND hWnd, const Column* columns, int columnCount);

	template<int ItemCount>
	HRESULT addItems(const CVar(&items)[ItemCount], LPVOID itemData = nullptr);
	HRESULT addItems(const CVar* items, int itemCount, LPVOID itemData = nullptr);

	int getSelectedIndex(int iStart = -1) const;
	std::tstring getItemText(int iItem, int iSubItem = 0) const;
	LPVOID getItemData(int iItem) const;

	HRESULT copy();
	HRESULT clear();

	void resize(int width, int height);

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
inline HRESULT CReportView::addItems(const CVar(&items)[ItemCount], LPVOID itemData /*= nullptr*/)
{
	return addItems(&items[0], ItemCount, itemData);
}
