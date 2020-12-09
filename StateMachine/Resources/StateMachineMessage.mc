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
����I�����܂����B
.

MessageId=
SymbolicName=TSM_S_DONE
Language=English
Nothing to do(S_FALSE).
.

Language=Japanese
�K�v�ȏ����͂���܂���B
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
���[�J�[�X���b�h���I�����܂����B
.

MessageId=
SymbolicName=TSM_S_SHUT_DOWN
Language=English
StateMachine::shutdown has been completed.
.

Language=Japanese
�V���b�g�_�E�����܂����B
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
�^�C�}�[�͒�~���܂����B
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
StateMachine::setup() �͎��s�ς݂ł�
.

MessageId=
SymbolicName=TSM_E_SETUP_HAS_NOT_BEEN_MADE
Language=English
StateMachine::setup() has not been completed on this context.
.

Language=Japanese
StateMachine::setup() �͖����s�ł��B
.

MessageId=
SymbolicName=TSM_E_CREATE_DISPATCHER
Language=English
Failed to create AsyncDispatcher.
.

Language=Japanese
AsyncDispatcher ���쐬�ł��܂���ł����B
.

MessageId=
SymbolicName=TSM_E_GET_DISPATCHER
Language=English
Failed to get AsyncDispatcher.
.

Language=Japanese
AsyncDispatcher ���擾�ł��܂���ł����B
.

MessageId=
SymbolicName=TSM_E_NOT_SUPPORTED
Language=English
This method is not supported by StateMachine.
Call method of AsyncStateMachine instead.
.

Language=Japanese
���\�b�h�� StateMachine �ŃT�|�[�g����Ă��܂���B
AsyncStatemachine �̃��\�b�h���g�p���Ă��������B
.

MessageId=
SymbolicName=TSM_E_WAIT_READY_BY_UNKNOWN_REASON
Language=English
Failed to wait ready due to unknown error.
.

Language=Japanese
���f�B�[�҂��ŃG���[���������܂����B
.

MessageId=
SymbolicName=TSM_E_WAIT_EVENT_BY_UNKNOWN_REASON
Language=English
Failed to wait Event due to unknown error.
.

Language=Japanese
�C�x���g�҂��ŃG���[���������܂����B
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
�R�[���o�b�N�̏I���҂��Ń^�C���A�E�g�������������߁A�^�C�}�[���L�����Z���ł��܂���ł����B
.

MessageId=
SymbolicName=TSM_E_CANCEL_TIMER_BY_UNKNOWN_REASON
Language=English
Failed to Cancel timer due to unknown error when waiting for timer callback to exit.
.

Language=Japanese
�R�[���o�b�N�̏I���҂��ŃG���[�������������߁A�^�C�}�[���L�����Z���ł��܂���ł����B
.

MessageId=
SymbolicName=TSM_E_NULL_TIMER_CLIENT
Language=English
TimerClient is not specified.
.

Language=Japanese
TimerClient ���w�肳��Ă��܂���B
.

MessageId=
SymbolicName=TSM_E_TIMER_HAS_BEEN_CREATRED
Language=English
Timer has been created already on this Event.
.

Language=Japanese
Event �ɑ΂��ă^�C�}�[�͊��ɍ쐬����Ă��܂��B
.

MessageId=
SymbolicName=TSM_E_WAIT_TIMEOUT
Language=English
Failed to wait for timeout.
.

Language=Japanese
�^�C���A�E�g�҂������s���܂����B
.
