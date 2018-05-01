#pragma once

#define HR_ERROR(msg, hr) checkHResult(hr, msg, _T(__FILE__), __LINE__)
#define HR_ASSERT(exp, hr) do { auto _hr(HR_EXPECT(exp, hr)); if(FAILED(_hr)) return _hr; } while(false)
#define HR_EXPECT(exp, hr) checkHResult((exp) ? S_OK : hr, _T(#exp), _T(__FILE__), __LINE__)
#define HR_ASSERT_OK(exp) do { auto _hr(HR_EXPECT_OK(exp)); if(FAILED(_hr)) return _hr; } while(false)
#define HR_EXPECT_OK(exp) checkHResult(exp, _T(#exp), _T(__FILE__), __LINE__)
#define WIN32_ASSERT(exp) HR_ASSERT(exp, HRESULT_FROM_WIN32(GetLastError()))
#define WIN32_EXPECT(exp) HR_EXPECT(exp, HRESULT_FROM_WIN32(GetLastError()))

HRESULT checkHResult(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);
