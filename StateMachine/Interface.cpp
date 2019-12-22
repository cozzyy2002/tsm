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

/*static*/ int IEvent::s_memoryWeightMByte = 0;
/*static*/ int IState::s_memoryWeightMByte = 0;

IEvent::IEvent()
	: m_memoryWeight(s_memoryWeightMByte ? new MByteUnit[s_memoryWeightMByte] : nullptr)
{

}

IEvent::~IEvent()
{
	if(m_memoryWeight) { delete [] m_memoryWeight; }
}

IState::IState()
	: m_memoryWeight(s_memoryWeightMByte ? new MByteUnit[s_memoryWeightMByte] : nullptr)
{

}

IState::~IState()
{
	if(m_memoryWeight) { delete[] m_memoryWeight; }
}
