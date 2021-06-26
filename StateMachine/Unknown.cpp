#include <StateMachine/stdafx.h>
#include <StateMachine/Unknown.h>

using namespace tsm;

std::tstring Unknown::toString() const
{
	CA2T str(typeid(*this).name());
	return (LPCTSTR)str;
}
