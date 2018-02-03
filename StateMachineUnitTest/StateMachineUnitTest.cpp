// StateMachineUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>
#include <StateMachine/StateMachine.h>

#include "Mocks.h"

using namespace testing;

template<class T>
static LPCSTR getObjectName(T* obj) { return typeid(*obj).name(); }

class StateMachineUnitTest : public Test
{
public:
	class Testee : public tsm::StateMachine {};

	~StateMachineUnitTest() {
		// Prevent Unknown::Release() method of MockState from being called after deleting.
		mockContext.m_currentState.Release();
	}

	Testee testee;
	MockContext mockContext;
	MockEvent mockEvent;
	MockState mockState0, mockState1;
};

// -------------------------
class StateMachineSetupUnitTest : public StateMachineUnitTest
{
public:
};

// StateMachine::setup(Event* = nullptr)
TEST_F(StateMachineSetupUnitTest, 0)
{
	EXPECT_CALL(mockState0, entry(&mockContext, nullptr, nullptr)).WillOnce(Return(S_OK));

	testee.setup(&mockContext, &mockState0);

	ASSERT_EQ(&mockState0, mockContext.m_currentState);
	EXPECT_FALSE(mockState0.deleted());
}

// StateMachine::setup(Event* = event)
TEST_F(StateMachineSetupUnitTest, 1)
{
	EXPECT_CALL(mockState0, entry(&mockContext, &mockEvent, nullptr)).WillOnce(Return(S_OK));

	testee.setup(&mockContext, &mockState0, &mockEvent);

	ASSERT_EQ(&mockState0, mockContext.m_currentState);
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

// -------------------------
class StateMachineEventUnitTest : public StateMachineUnitTest
{
public:
	void SetUp() {
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, nullptr)).WillOnce(Return(S_OK));
		testee.setup(&mockContext, &mockState0);
		ASSERT_EQ(&mockState0, mockContext.m_currentState);
		EXPECT_FALSE(mockState0.deleted());
	}
};

// No state transition occurs.
TEST_F(StateMachineEventUnitTest, 0)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&mockContext, &mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

// State transition occurs.
TEST_F(StateMachineEventUnitTest, 1)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
	EXPECT_CALL(mockState0, exit(&mockContext, &mockEvent, &mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState1, entry(&mockContext, &mockEvent, &mockState0)).WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&mockContext, &mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_TRUE(mockState0.deleted());
	EXPECT_FALSE(mockState1.deleted());
}
