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
	class Testee : public tsm::StateMachine
	{};

	void SetUp() {
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, nullptr)).WillOnce(Return(S_OK));
		testee.setup(&mockContext, &mockState0);
		ASSERT_EQ(&mockState0, mockContext.m_currentState);
		EXPECT_FALSE(mockState0.deleted());
	}
	void TearDown() {
		// Prevent Unknown::Release() method of MockState from being called after deleting.
		mockContext.m_currentState.Release();
	}

	Testee testee;
	MockContext mockContext;
	MockEvent mockEvent;
	MockState mockState0, mockState1;
};

TEST_F(StateMachineUnitTest, 0)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, _)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);
	EXPECT_CALL(mockState1, entry(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&mockContext, &mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

TEST_F(StateMachineUnitTest, 1)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, _)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState0, exit(&mockContext, &mockEvent, &mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState1, entry(&mockContext, &mockEvent, &mockState0)).WillOnce(Return(S_OK));
}
