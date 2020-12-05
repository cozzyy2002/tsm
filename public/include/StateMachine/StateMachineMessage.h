 // Message file
 // See https://docs.microsoft.com/en-us/windows/win32/eventlog/sample-message-text-file for message file sample.
 // The following are message definitions.
 //
 // --------------------------------------------------
 // StateMachine success
 // --------------------------------------------------
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-+-+-+-+---------------------+-------------------------------+
//  |S|R|C|N|r|    Facility         |               Code            |
//  +-+-+-+-+-+---------------------+-------------------------------+
//
//  where
//
//      S - Severity - indicates success/fail
//
//          0 - Success
//          1 - Fail (COERROR)
//
//      R - reserved portion of the facility code, corresponds to NT's
//              second severity bit.
//
//      C - reserved portion of the facility code, corresponds to NT's
//              C field.
//
//      N - reserved portion of the facility code. Used to indicate a
//              mapped NT status value.
//
//      r - reserved portion of the facility code. Reserved for internal
//              use. Used to indicate HRESULT values that are not status
//              values, but are instead message ids for display strings.
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define TSM_FACILITY_STATE_MACHINE       0x101
#define TSM_FACILITY_TIMER               0x102


//
// Define the severity codes
//
#define TSM_HR_SEVERITY_SUCCESS          0x0
#define TSM_HR_SEVERITY_FAILURE          0x2


//
// MessageId: TSM_S_DONE
//
// MessageText:
//
// Nothing to do.
//
#define TSM_S_DONE                       ((DWORD)0x01010001L)

//
// MessageId: TSM_S_NO_WORKER_THREAD
//
// MessageText:
//
// Worker thiread has been terminated.
//
#define TSM_S_NO_WORKER_THREAD           ((DWORD)0x01010002L)

//
// MessageId: TSM_S_SHUTTED_DOWN
//
// MessageText:
//
// StateMachine::shutdown has been completed.
//
#define TSM_S_SHUTTED_DOWN               ((DWORD)0x01010003L)

 // --------------------------------------------------
 // StateMachine error
 // --------------------------------------------------
//
// MessageId: TSM_E_SETUP_HAS_BEEN_MADE
//
// MessageText:
//
// StateMachine::setup() has been completed on this Context.
//
#define TSM_E_SETUP_HAS_BEEN_MADE        ((DWORD)0x81010001L)

//
// MessageId: TSM_E_SETUP_HAS_NOT_BEEN_MADE
//
// MessageText:
//
// StateMachine::setup() has not been completed on this context.
//
#define TSM_E_SETUP_HAS_NOT_BEEN_MADE    ((DWORD)0x81010002L)

//
// MessageId: TSM_E_CREATE_DISPATCHER
//
// MessageText:
//
// Failed to create AsyncDispatcher.
//
#define TSM_E_CREATE_DISPATCHER          ((DWORD)0x81010003L)

//
// MessageId: TSM_E_GET_DISPATCHER
//
// MessageText:
//
// Failed to get AsyncDispatcher.
//
#define TSM_E_GET_DISPATCHER             ((DWORD)0x81010004L)

//
// MessageId: TSM_E_NOT_SUPPORTED
//
// MessageText:
//
// This method is not supported by StateMachine.
// Call method of AsyncStateMachine instead.
//
#define TSM_E_NOT_SUPPORTED              ((DWORD)0x81010005L)

//
// MessageId: TSM_E_WAIT_READY_BY_UNKNOWN_REASON
//
// MessageText:
//
// Failed to wait ready due to unknown error.
//
#define TSM_E_WAIT_READY_BY_UNKNOWN_REASON ((DWORD)0x81010006L)

//
// MessageId: TSM_E_WAIT_EVENT_BY_UNKNOWN_REASON
//
// MessageText:
//
// Failed to wait Event due to unknown error.
//
#define TSM_E_WAIT_EVENT_BY_UNKNOWN_REASON ((DWORD)0x81010007L)

 // --------------------------------------------------
 // Timer error
 // --------------------------------------------------
//
// MessageId: TSM_E_CANCEL_TIMER_BY_TIMEOUT
//
// MessageText:
//
// Failed to Cancel timer due to timeout to wait for timer callback to exit.
//
#define TSM_E_CANCEL_TIMER_BY_TIMEOUT    ((DWORD)0x81020001L)

//
// MessageId: TSM_E_CANCEL_TIMER_BY_UNKNOWN_REASON
//
// MessageText:
//
// Failed to Cancel timer due to unknown error when waiting for timer callback to exit.
//
#define TSM_E_CANCEL_TIMER_BY_UNKNOWN_REASON ((DWORD)0x81020002L)

//
// MessageId: TSM_E_NULL_TIMER_CLIENT
//
// MessageText:
//
// TimerClient is not specified.
//
#define TSM_E_NULL_TIMER_CLIENT          ((DWORD)0x81020003L)

//
// MessageId: TSM_E_TIMER_HAS_BEEN_CREATRED
//
// MessageText:
//
// Timer has been created already on this Event.
//
#define TSM_E_TIMER_HAS_BEEN_CREATRED    ((DWORD)0x81020004L)

//
// MessageId: TSM_E_WAIT_TIMEOUT
//
// MessageText:
//
// Failed to wait for timeout.
//
#define TSM_E_WAIT_TIMEOUT               ((DWORD)0x81020005L)

