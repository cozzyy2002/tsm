#include "stdafx.h"

#include "StateMachine.NET.h"
#include "NativeObjects.h"
#include "GenericObject.h"

using namespace tsm_NET;
using namespace tsm_NET::common;
using namespace System::Diagnostics;

///*static*/ event EventHandler<IStateMonitor::AssertFailedEventArgs<HResult>^>^ IStateMonitor::AssertFailedEvent;

//-------------- Managed TimerClient class. --------------------//

::CultureInfo^ Error::CultureInfo::get()
{
	return s_cultureInfo;
}

void Error::CultureInfo::set(::CultureInfo^ value)
{
	s_cultureInfo = value;
}

Error::Error(HRESULT hr)
	: m_hr(hr), m_message(nullptr)
{
}

Error::Error(tsm_NET::HResult hr)
	: m_hr((HRESULT)hr), m_message(nullptr)
{
}

HRESULT Error::HResult::get()
{
	return m_hr;
}

String^ Error::Message::get()
{
	if(m_message == nullptr) {
		auto hModule = tsm::GetStateMachineModule();
		auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		auto langId = (s_cultureInfo != nullptr) ? s_cultureInfo->LCID : 0;
		LPTSTR message;
		va_list args;
		FormatMessage(flags, (LPCVOID)hModule, m_hr, langId, (LPTSTR)&message, 100, &args);
		m_message = gcnew String(message);
		LocalFree(message);
	}
	return m_message;
}
