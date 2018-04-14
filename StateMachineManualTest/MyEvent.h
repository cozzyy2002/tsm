#pragma once

#include <StateMachine/Event.h>
#include "MyObject.h"

class MyEvent : public tsm::Event, public MyObject
{
public:
};
