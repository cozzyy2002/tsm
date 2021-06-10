#include <StateMachine/stdafx.h>

#include <StateMachine/Assert.h>

// Default OnAssertFailedProc
// Formats error message and pass the message to the writer.
/*static*/ tsm::Assert::OnAssertFailedProc tsm::Assert::onAssertFailedProc = [](HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	TCHAR msg[1000];
	_stprintf_s(msg, _T("'%s' failed: HRESULT=0x%08x at:\n%s:%d"), exp, hr, sourceFile, line);
	onAssertFailedWriter(msg);
};

// Default OnAssertFailedWriter
// Writes the message to the debugger.
/*static*/ tsm::Assert::OnAssertFailedWriter tsm::Assert::onAssertFailedWriter = [](LPCTSTR msg)
{
	OutputDebugString(msg);
	OutputDebugString(_T("\n"));
};

/*static*/ HRESULT tsm::Assert::checkHResult(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	if(FAILED(hr)) {
		onAssertFailedProc(hr, exp, sourceFile, line);
	}
	return hr;
}
