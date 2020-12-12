// This file might be included more than once.
//#pragma once

#include <StateMachine/StateMachineMessage.h>
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

	NoWorkerThread = TSM_S_NO_WORKER_THREAD,
	ShutDown = TSM_S_SHUT_DOWN,

	TimerIsStopped = TSM_S_TIMER_IS_STOPPED,

	SetupHasBeenMade = (HRESULT)TSM_E_SETUP_HAS_BEEN_MADE,
	SetupHasNotBeenMade = (HRESULT)TSM_E_SETUP_HAS_NOT_BEEN_MADE,
	CreateDispatcher = (HRESULT)TSM_E_CREATE_DISPATCHER,
	GetDispatcher = (HRESULT)TSM_E_GET_DISPATCHER,
	NotSupported = (HRESULT)TSM_E_NOT_SUPPORTED,
	WaitReadyByUnknownReason = (HRESULT)TSM_E_WAIT_READY_BY_UNKNOWN_REASON,
	WaitEventByUnknownReason = (HRESULT)TSM_E_WAIT_EVENT_BY_UNKNOWN_REASON,
	CancelTimerByTimeout = (HRESULT)TSM_E_CANCEL_TIMER_BY_TIMEOUT,
	CancelTimerByUnknownReason = (HRESULT)TSM_E_CANCEL_TIMER_BY_UNKNOWN_REASON,
	NullTimerClient = (HRESULT)TSM_E_NULL_TIMER_CLIENT,
	TimerHasBeenCreated = (HRESULT)TSM_E_TIMER_HAS_BEEN_CREATRED,
	WaitTimeout = (HRESULT)TSM_E_WAIT_TIMEOUT,

	NotImpl = E_NOTIMPL,
	IllegalMethodCall = E_ILLEGAL_METHOD_CALL,
	Abort = E_ABORT,
	UnExpected = E_UNEXPECTED,
};
