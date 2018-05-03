#include "stdafx.h"
#include "ClipBoard.h"

#include <StateMachine/Assert.h>

CClipBoard::CClipBoard() : opened(FALSE), hMem(NULL)
{
}

CClipBoard::~CClipBoard()
{
	if(opened) WIN32_EXPECT(CloseClipboard());
	if(hMem) WIN32_EXPECT(!GlobalFree(hMem));
}

HRESULT CClipBoard::open(HWND hWnd)
{
	WIN32_ASSERT(opened = OpenClipboard(hWnd));
	WIN32_ASSERT(EmptyClipboard());
	return S_OK;
}

// Allocates global memory, copies src to the memory and copies the memory to clip board.
// If src is string, size should include terminating null character.
HRESULT CClipBoard::copy(UINT uFormat, LPCVOID src, size_t size)
{
	if(hMem) WIN32_ASSERT(!GlobalFree(hMem));
	WIN32_ASSERT(hMem = GlobalAlloc(GMEM_MOVEABLE, size));
	LPVOID dest;
	WIN32_ASSERT(dest = GlobalLock(hMem));
	CopyMemory(dest, src, size);
	WIN32_ASSERT(GlobalUnlock(hMem));
	WIN32_ASSERT(SetClipboardData(uFormat, hMem));
	// Prevent global memory from being freed.
	hMem = NULL;
	return S_OK;
}
