#include <StateMachine/stdafx.h>
#include <StateMachine/Interface.h>

using namespace tsm;

namespace tsm
{
struct MByteUnit
{
	BYTE _[1024][1024];
};

}

/*static*/ LONG IEvent::s_sequenceNumber = 0;
/*static*/ int IEvent::s_memoryWeightMByte = 0;
/*static*/ LONG IState::s_sequenceNumber = 0;
/*static*/ int IState::s_memoryWeightMByte = 0;

IEvent::IEvent()
	: m_sequenceNumber(InterlockedIncrement(&s_sequenceNumber))
	, m_memoryWeight(s_memoryWeightMByte ? new MByteUnit[s_memoryWeightMByte] : nullptr)
{

}

IEvent::~IEvent()
{
	if(m_memoryWeight) { delete [] m_memoryWeight; }
}

IState::IState()
	: m_sequenceNumber(InterlockedIncrement(&s_sequenceNumber))
	, m_memoryWeight(s_memoryWeightMByte ? new MByteUnit[s_memoryWeightMByte] : nullptr)
{

}

IState::~IState()
{
	if(m_memoryWeight) { delete[] m_memoryWeight; }
}
