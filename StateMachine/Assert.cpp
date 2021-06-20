#include <StateMachine/stdafx.h>

#include <StateMachine/Assert.h>

using namespace tsm;


// Default OnAssertFailedProc
// Formats error message and pass the message to the writer.
/*static*/ void Assert::defaultAssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	TCHAR msg[1000];
	_stprintf_s(msg, _T("'%s' failed: HRESULT=0x%08x at:\n%s:%d"), exp, hr, sourceFile, line);
	auto writer = Assert::onAssertFailedWriter ? Assert::onAssertFailedWriter : defaultAssertFailedWriter;
	writer(msg);
}

// Default OnAssertFailedWriter
// Writes the message to the debugger.
/*static*/ void Assert::defaultAssertFailedWriter(LPCTSTR msg)
{
	OutputDebugString(msg);
	OutputDebugString(_T("\n"));
}

/*static*/ Assert::OnAssertFailedProc Assert::onAssertFailedProc = nullptr;
/*static*/ Assert::OnAssertFailedWriter Assert::onAssertFailedWriter = nullptr;

/*static*/ HRESULT Assert::checkHResult(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	if(FAILED(hr)) {
		auto proc = onAssertFailedProc ? onAssertFailedProc : defaultAssertFailedProc;
		proc(hr, exp, sourceFile, line);
	}
	return hr;
}
