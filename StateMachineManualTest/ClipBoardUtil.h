#pragma once

class CSafeClipBoard
{
public:
	CSafeClipBoard(HWND hWnd);
	~CSafeClipBoard();
	HRESULT open();

protected:
	BOOL opened;
	HWND hWnd;
};

// Manages global heap for clip board.
class CClipBoardBuffer
{
public:
	CClipBoardBuffer();
	~CClipBoardBuffer();

	HRESULT copy(LPCVOID src, size_t size);

	// Returns global memory handle.
	HGLOBAL get() { return hMem; }

	// Detaches global memory.
	// Call this method after clip board operation completed to prevent global memory from being freed.
	void detach() { hMem = NULL; }

protected:
	HGLOBAL hMem;
};
