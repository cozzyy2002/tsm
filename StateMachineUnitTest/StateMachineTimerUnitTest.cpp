#include "stdafx.h"

#include <StateMachine/TimerClient.h>
#include "../StateMachine/Handles.h"
#include "Mocks.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("UnitTest.StateMachineTimerUnitTest"));
using namespace testing;

template<class C>
class StateMachineTimerUnitTest : public Test
{
public:
	using UnitTestBase = StateMachineTimerUnitTest<C>;

	void SetUp() {
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, nullptr)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
		ASSERT_EQ(S_OK, mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, mockContext.shutdown());
		EXPECT_EQ(nullptr, mockContext._getCurrentState());
	}

	tsm::TimerHandle* getTimerHandle(tsm::TimerClient& timerClient) { return  timerClient._getHandle(); }

	C mockContext;
	MockEvent e0, e1;
	MockState<C> mockState0, mockState1;
};

class StateMachineTriggerEventUnitTest : public StateMachineTimerUnitTest<MockAsyncContext>
{
public:
	void SetUp() { UnitTestBase::SetUp(); }
	void TearDown() { UnitTestBase::TearDown(); }
};

// Context timer.
TEST_F(StateMachineTriggerEventUnitTest, 0)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(S_OK));

	e0.setTimer(&mockContext, 50);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));

	ASSERT_EQ(1, getTimerHandle(mockContext)->timers.size());
	Sleep(110);
	ASSERT_EQ(0, getTimerHandle(mockContext)->timers.size());
	EXPECT_TRUE(e0.deleted());
}

// State timer
TEST_F(StateMachineTriggerEventUnitTest, 1)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(S_OK));

	e0.setTimer(&mockState0, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));

	ASSERT_EQ(1, getTimerHandle(mockState0)->timers.size());
	Sleep(110);
	ASSERT_EQ(0, getTimerHandle(mockState0)->timers.size());
	EXPECT_TRUE(e0.deleted());
}

// Cancel one-shot state timer
TEST_F(StateMachineTriggerEventUnitTest, 2)
{
	EXPECT_CALL(mockState0, handleEvent(_, _, _)).Times(0);

	e0.setTimer(&mockState0, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));

	ASSERT_EQ(1, getTimerHandle(mockState0)->timers.size());
	Sleep(50);
	ASSERT_EQ(S_OK, mockState0.cancelEventTimer(&e0));
	ASSERT_EQ(0, getTimerHandle(mockState0)->timers.size());
	EXPECT_TRUE(e0.deleted());
}

// Cancel interval state timer
TEST_F(StateMachineTriggerEventUnitTest, 3)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, _)).Times(2);

	e0.setTimer(&mockState0, 50, 30);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));

	ASSERT_EQ(1, getTimerHandle(mockState0)->timers.size());
	Sleep(100);
	ASSERT_EQ(1, getTimerHandle(mockState0)->timers.size());
	ASSERT_EQ(S_OK, mockState0.cancelEventTimer(&e0));
	ASSERT_EQ(0, getTimerHandle(mockState0)->timers.size());
	EXPECT_TRUE(e0.deleted());
}

// Cancel event timer of state on State::exit()
TEST_F(StateMachineTriggerEventUnitTest, 4)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
	// Interval timer of e1 should be canceled.
	EXPECT_CALL(mockState0, handleEvent(_, &e1, _)).Times(2);
	EXPECT_CALL(mockState0, exit(&mockContext, &e0, &mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState1, entry(&mockContext, &e0, &mockState0)).WillOnce(Return(S_OK));

	// Start interval timer to be canceled.
	e1.setTimer(&mockState0, 50, 30);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e1));

	ASSERT_EQ(1, getTimerHandle(mockState0)->timers.size());
	// mockState0 -> mockState1 -> Cancel delayed event e1 of mockState0.
	Sleep(100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));
	Sleep(10);
	ASSERT_EQ(&mockState1, mockContext.getCurrentState());
	ASSERT_EQ(0, getTimerHandle(mockState0)->timers.size());
}
