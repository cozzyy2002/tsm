// This file might be included more than once.
//#pragma once

/*
 * Definition of HResult for native classes.
 *
 * This file is intended to be included in namespace tsm_NET and tsm_NET.Generic
 * to define HResult type in each namespace.
 */
public enum class HResult : HRESULT
{
	Ok = S_OK,
	False = S_FALSE,

	NotImpl = E_NOTIMPL,
	IllegalMethodCall = E_ILLEGAL_METHOD_CALL,
	UnExpected = E_UNEXPECTED,
};
