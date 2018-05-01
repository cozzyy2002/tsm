#include "stdafx.h"
#include "ClipBoardUtil.h"

#include <StateMachine/Assert.h>

CSafeClipBoard::CSafeClipBoard(HWND hWnd) : opened(FALSE), hWnd(hWnd)
{
}

CSafeClipBoard::~CSafeClipBoard()
{
	if(opened) WIN32_EXPECT(CloseClipboard());
}

HRESULT CSafeClipBoard::open()
{
	WIN32_ASSERT(opened = OpenClipboard(hWnd));
	WIN32_ASSERT(EmptyClipboard());
	return S_OK;
}

CClipBoardBuffer::CClipBoardBuffer() : hMem(NULL)
{
}

CClipBoardBuffer::~CClipBoardBuffer()
{
	if(hMem) WIN32_EXPECT(!GlobalFree(hMem));
}

// Allocates global memory and copies src to the memory
// If src is string, size should include terminating null character.
HRESULT CClipBoardBuffer::copy(LPCVOID src, size_t size)
{
	if(hMem) WIN32_ASSERT(!GlobalFree(hMem));
	WIN32_ASSERT(hMem = GlobalAlloc(GMEM_MOVEABLE, size));
	LPVOID dest;
	WIN32_ASSERT(dest = GlobalLock(hMem));
	CopyMemory(dest, src, size);
	WIN32_ASSERT(GlobalUnlock(hMem));
	return S_OK;
}
