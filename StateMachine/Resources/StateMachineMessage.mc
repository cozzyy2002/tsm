;// Message file
;// See https://docs.microsoft.com/en-us/windows/win32/eventlog/sample-message-text-file for message file sample.

; #pragma once

MessageIdTypedef=DWORD

SeverityNames=(
	Success=0x0:TSM_HR_SEVERITY_SUCCESS
	Failure=0x2:TSM_HR_SEVERITY_FAILURE
)

FacilityNames=(
	System=0:TSM_FACILITY_SYSTEM
	StateMachne=0x101:TSM_FACILITY_STATE_MACHINE
	Timer=0x102:TSM_FACILITY_TIMER
)

LanguageNames=(English=0x409:MSG00409)
LanguageNames=(Japanese=0x411:MSG00411)

;// The following are message definitions.


;// --------------------------------------------------
;// System success
;// --------------------------------------------------
MessageId=0x0
Severity=Success
Facility=System
SymbolicName=TSM_S_OK
Language=English
OK(S_OK).
.

Language=Japanese
正常終了しました。
.

MessageId=
SymbolicName=TSM_S_DONE
Language=English
Nothing to do(S_FALSE).
.

Language=Japanese
必要な処理はありません。
.

;// --------------------------------------------------
;// StateMachine success
;// --------------------------------------------------
MessageId=0x1
Severity=Success
Facility=StateMachne
SymbolicName=TSM_S_NO_WORKER_THREAD
Language=English
Worker thiread has been terminated.
.

Language=Japanese
ワーカースレッドが終了しました。
.

MessageId=
SymbolicName=TSM_S_SHUT_DOWN
Language=English
StateMachine::shutdown has been completed.
.

Language=Japanese
シャットダウンしました。
.

;// --------------------------------------------------
;// Timer success
;// --------------------------------------------------

MessageId=
Facility=Timer
SymbolicName=TSM_S_TIMER_IS_STOPPED
Language=English
Timer is not set or is stopped already.
.

Language=Japanese
タイマーは停止しました。
.


;// --------------------------------------------------
;// StateMachine error
;// --------------------------------------------------

MessageId=0x1
Severity=Failure
Facility=StateMachne
SymbolicName=TSM_E_SETUP_HAS_BEEN_MADE
Language=English
StateMachine::setup() has been completed on this Context.
.

Language=Japanese
StateMachine::setup() は実行済みです
.

MessageId=
SymbolicName=TSM_E_SETUP_HAS_NOT_BEEN_MADE
Language=English
StateMachine::setup() has not been completed on this context.
.

Language=Japanese
StateMachine::setup() は未実行です。
.

MessageId=
SymbolicName=TSM_E_CREATE_DISPATCHER
Language=English
Failed to create AsyncDispatcher.
.

Language=Japanese
AsyncDispatcher が作成できませんでした。
.

MessageId=
SymbolicName=TSM_E_GET_DISPATCHER
Language=English
Failed to get AsyncDispatcher.
.

Language=Japanese
AsyncDispatcher が取得できませんでした。
.

MessageId=
SymbolicName=TSM_E_NOT_SUPPORTED
Language=English
This method is not supported by StateMachine.
Call method of AsyncStateMachine instead.
.

Language=Japanese
メソッドが StateMachine でサポートされていません。
AsyncStatemachine のメソッドを使用してください。
.

MessageId=
SymbolicName=TSM_E_WAIT_READY_BY_UNKNOWN_REASON
Language=English
Failed to wait ready due to unknown error.
.

Language=Japanese
レディー待ちでエラーが発生しました。
.

MessageId=
SymbolicName=TSM_E_WAIT_EVENT_BY_UNKNOWN_REASON
Language=English
Failed to wait Event due to unknown error.
.

Language=Japanese
イベント待ちでエラーが発生しました。
.


;// --------------------------------------------------
;// Timer error
;// --------------------------------------------------

MessageId=
Severity=Failure
Facility=Timer
SymbolicName=TSM_E_CANCEL_TIMER_BY_TIMEOUT
Language=English
Failed to Cancel timer due to timeout to wait for timer callback to exit.
.

Language=Japanese
コールバックの終了待ちでタイムアウトが発生したため、タイマーをキャンセルできませんでした。
.

MessageId=
SymbolicName=TSM_E_CANCEL_TIMER_BY_UNKNOWN_REASON
Language=English
Failed to Cancel timer due to unknown error when waiting for timer callback to exit.
.

Language=Japanese
コールバックの終了待ちでエラーが発生したため、タイマーをキャンセルできませんでした。
.

MessageId=
SymbolicName=TSM_E_NULL_TIMER_CLIENT
Language=English
TimerClient is not specified.
.

Language=Japanese
TimerClient が指定されていません。
.

MessageId=
SymbolicName=TSM_E_TIMER_HAS_BEEN_CREATRED
Language=English
Timer has been created already on this Event.
.

Language=Japanese
Event に対してタイマーは既に作成されています。
.

MessageId=
SymbolicName=TSM_E_WAIT_TIMEOUT
Language=English
Failed to wait for timeout.
.

Language=Japanese
タイムアウト待ちが失敗しました。
.
