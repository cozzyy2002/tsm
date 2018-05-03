#pragma once

class CClipBoard
{
public:
	CClipBoard();
	~CClipBoard();
	HRESULT open(HWND hWnd);
	HRESULT copy(UINT uFormat, LPCVOID src, size_t size);

protected:
	BOOL opened;
	HGLOBAL hMem;
};
