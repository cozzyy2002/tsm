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
	using MockEvent_t = MockEvent<C>;

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
	auto getPendingEvents(tsm::TimerClient& timerClient) {
		auto eventVectors = timerClient.getPendingEvents();
		std::set<tsm::IEvent*> events;
		for(auto& event : eventVectors) {
			events.insert(event.p);
		}

		return events;
	}

	C mockContext;
	MockEvent_t e0, e1;
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
	EXPECT_CALL(e0, preHandle(&mockContext)).Times(1);
	EXPECT_CALL(e0, postHandle(&mockContext, S_OK)).Times(1);
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(S_OK));

	e0.setTimer(&mockContext, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));
	Sleep(50);
	auto events = getPendingEvents(mockContext);
	EXPECT_EQ(1, events.size());
	EXPECT_NE(events.end(), events.find(&e0));
	Sleep(100);
	events = getPendingEvents(mockContext);
	EXPECT_EQ(0, events.size());
	EXPECT_TRUE(e0.deleted());
}

// State timer
TEST_F(StateMachineTriggerEventUnitTest, 1)
{
	EXPECT_CALL(e0, preHandle(&mockContext)).Times(1);
	EXPECT_CALL(e0, postHandle(&mockContext, S_OK)).Times(1);
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(S_OK));

	e0.setTimer(&mockState0, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));
	Sleep(50);
	auto events = getPendingEvents(mockState0);
	EXPECT_EQ(1, events.size());
	EXPECT_NE(events.end(), events.find(&e0));
	Sleep(100);
	events = getPendingEvents(mockState0);
	EXPECT_EQ(0, events.size());
	EXPECT_TRUE(e0.deleted());
}

// Cancel one-shot state timer
TEST_F(StateMachineTriggerEventUnitTest, 2)
{
	EXPECT_CALL(e0, preHandle(_)).Times(0);
	EXPECT_CALL(e0, postHandle(_, _)).Times(0);
	EXPECT_CALL(mockState0, handleEvent(_, _, _)).Times(0);

	e0.setTimer(&mockState0, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));
	Sleep(50);
	auto events = getPendingEvents(mockState0);
	EXPECT_EQ(1, events.size());
	EXPECT_NE(events.end(), events.find(&e0));
	ASSERT_EQ(S_OK, mockState0.cancelEventTimer(&e0));
	events = getPendingEvents(mockState0);
	EXPECT_EQ(0, events.size());
	EXPECT_TRUE(e0.deleted());
	Sleep(500);
}

// Cancel interval state timer
TEST_F(StateMachineTriggerEventUnitTest, 3)
{
	EXPECT_CALL(e0, preHandle(&mockContext)).Times(2);
	EXPECT_CALL(e0, postHandle(&mockContext, S_OK)).Times(2);
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, _)).Times(2);

	e0.setTimer(&mockState0, 50, 30);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));

	auto events = getPendingEvents(mockState0);
	EXPECT_EQ(1, events.size());
	EXPECT_NE(events.end(), events.find(&e0));
	Sleep(100);
	events = getPendingEvents(mockState0);
	EXPECT_EQ(1, events.size());
	EXPECT_NE(events.end(), events.find(&e0));
	ASSERT_EQ(S_OK, mockState0.cancelEventTimer(&e0));
	events = getPendingEvents(mockState0);
	EXPECT_EQ(0, events.size());
	Sleep(10);
	EXPECT_TRUE(e0.deleted());
	Sleep(500);
}

// Cancel event timer of state on State::exit()
TEST_F(StateMachineTriggerEventUnitTest, 4)
{
	EXPECT_CALL(e0, preHandle(&mockContext)).Times(1);
	EXPECT_CALL(e0, postHandle(&mockContext, S_OK)).Times(1);
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
	// Interval timer of e1 should be canceled.
	EXPECT_CALL(e1, preHandle(&mockContext)).Times(2);
	EXPECT_CALL(e1, postHandle(&mockContext, S_OK)).Times(2);
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e1, _)).Times(2);
	EXPECT_CALL(mockState0, exit(&mockContext, &e0, &mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState1, entry(&mockContext, &e0, &mockState0)).WillOnce(Return(S_OK));

	// Start interval timer to be canceled.
	e1.setTimer(&mockState0, 50, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e1));

	auto events = getPendingEvents(mockState0);
	EXPECT_EQ(1, events.size());
	EXPECT_NE(events.end(), events.find(&e1));
	// mockState0 -> mockState1 -> Cancel delayed event e1 of mockState0.
	Sleep(200);
	EXPECT_FALSE(e1.deleted());
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));
	Sleep(200);
	ASSERT_EQ(&mockState1, mockContext.getCurrentState());
	events = getPendingEvents(mockState0);
	EXPECT_EQ(0, events.size());
	EXPECT_TRUE(e1.deleted());
}
