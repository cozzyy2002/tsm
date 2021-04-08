// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers



// TODO: reference additional headers your program requires here
#include <Unknwn.h>
#include <Shlwapi.h>
#include <winerror.h>

#include <typeinfo>
#include <string>
#include <memory>
#include <functional>
#include <atlbase.h>

namespace std {
#if defined(UNICODE)
using tstring = std::wstring;
#else
using tstring = std::string;
#endif
}

#if defined(tsm_STATE_MACHINE_EXPORTS)
#define tsm_STATE_MACHINE_EXPORT __declspec(dllexport)
#else
#define tsm_STATE_MACHINE_EXPORT __declspec(dllimport)
#endif
