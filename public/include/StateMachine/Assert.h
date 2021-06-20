#pragma once

#define HR_ERROR(msg, hr) tsm::Assert::checkHResult(hr, msg, _T(__FILE__), __LINE__)
#define HR_ASSERT(exp, hr) do { auto _hr(HR_EXPECT(exp, hr)); if(FAILED(_hr)) return _hr; } while(false)
#define HR_EXPECT(exp, hr) tsm::Assert::checkHResult((exp) ? S_OK : hr, _T(#exp), _T(__FILE__), __LINE__)
#define HR_ASSERT_OK(exp) do { auto _hr(HR_EXPECT_OK(exp)); if(FAILED(_hr)) return _hr; } while(false)
#define HR_EXPECT_OK(exp) tsm::Assert::checkHResult(exp, _T(#exp), _T(__FILE__), __LINE__)
#define WIN32_ASSERT(exp) HR_ASSERT(exp, HRESULT_FROM_WIN32(GetLastError()))
#define WIN32_EXPECT(exp) HR_EXPECT(exp, HRESULT_FROM_WIN32(GetLastError()))

namespace tsm
{

class tsm_STATE_MACHINE_EXPORT Assert
{
public:
	static HRESULT checkHResult(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);

	// Procedure that is called when error occurs in ASSERT/EXPECT macro.
	using OnAssertFailedProc = void (*)(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);
	static OnAssertFailedProc onAssertFailedProc;
	static void defaultAssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);

	// Procedure that is called by OnAssertFailedProc to write error message.
	using OnAssertFailedWriter = void (*)(LPCTSTR msg);
	static OnAssertFailedWriter onAssertFailedWriter;
	static void defaultAssertFailedWriter(LPCTSTR msg);
};

}
