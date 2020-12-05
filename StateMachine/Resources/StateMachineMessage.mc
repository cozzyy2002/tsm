; // Message file
; // See https://docs.microsoft.com/en-us/windows/win32/eventlog/sample-message-text-file for message file sample.

MessageIdTypedef=DWORD

SeverityNames=(
	Success=0x0:TSM_HR_SEVERITY_SUCCESS
	Failure=0x2:TSM_HR_SEVERITY_FAILURE
)

FacilityNames=(
	StateMachne=0x101:TSM_FACILITY_STATE_MACHINE
	Timer=0x102:TSM_FACILITY_TIMER
)

LanguageNames=(English=0x409:MSG00409)
LanguageNames=(Japanese=0x411:MSG00411)

; // The following are message definitions.


; //

; // --------------------------------------------------
; // StateMachine success
; // --------------------------------------------------
MessageId=0x1
Severity=Success
Facility=StateMachne
SymbolicName=TSM_S_DONE
Language=English
Nothing to do.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_S_NO_WORKER_THREAD
Language=English
Worker thiread has been terminated.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_S_SHUTTED_DOWN
Language=English
StateMachine::shutdown has been completed.
.

Language=Japanese
日本語
.


; // --------------------------------------------------
; // StateMachine error
; // --------------------------------------------------

MessageId=0x1
Severity=Failure
Facility=StateMachne
SymbolicName=TSM_E_SETUP_HAS_BEEN_MADE
Language=English
StateMachine::setup() has been completed on this Context.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_SETUP_HAS_NOT_BEEN_MADE
Language=English
StateMachine::setup() has not been completed on this context.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_CREATE_DISPATCHER
Language=English
Failed to create AsyncDispatcher.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_GET_DISPATCHER
Language=English
Failed to get AsyncDispatcher.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_NOT_SUPPORTED
Language=English
This method is not supported by StateMachine.
Call method of AsyncStateMachine instead.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_WAIT_READY_BY_UNKNOWN_REASON
Language=English
Failed to wait ready due to unknown error.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_WAIT_EVENT_BY_UNKNOWN_REASON
Language=English
Failed to wait Event due to unknown error.
.

Language=Japanese
日本語
.


; // --------------------------------------------------
; // Timer error
; // --------------------------------------------------

MessageId=
Severity=Failure
Facility=Timer
SymbolicName=TSM_E_CANCEL_TIMER_BY_TIMEOUT
Language=English
Failed to Cancel timer due to timeout to wait for timer callback to exit.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_CANCEL_TIMER_BY_UNKNOWN_REASON
Language=English
Failed to Cancel timer due to unknown error when waiting for timer callback to exit.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_NULL_TIMER_CLIENT
Language=English
TimerClient is not specified.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_TIMER_HAS_BEEN_CREATRED
Language=English
Timer has been created already on this Event.
.

Language=Japanese
日本語
.

MessageId=
SymbolicName=TSM_E_WAIT_TIMEOUT
Language=English
Failed to wait for timeout.
.

Language=Japanese
日本語
.
