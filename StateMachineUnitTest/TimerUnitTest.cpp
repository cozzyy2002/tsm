#include "stdafx.h"

#include <StateMachine/StateMachineMessage.h>
#include "../StateMachine/Handles.h"
#include "Mocks.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("UnitTest.TimerUnitTest"));
using namespace testing;

template<class C>
class TimerUnitTest : public Test
{
public:
	using UnitTestBase = TimerUnitTest<C>;
	using MockEvent_t = MockEvent<C>;

	static const int CANCEL_TIMER_TIMEOUT = 100;

	void SetUp() {
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, nullptr)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
		ASSERT_EQ(S_OK, mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, mockContext.shutdown());
		EXPECT_EQ(nullptr, mockContext._getCurrentState());
	}

	auto getPendingEvents(tsm::ITimerOwner& timerOwner) {
		auto eventVectors = timerOwner.getPendingEvents();
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

class TriggerEventUnitTest : public TimerUnitTest<MockAsyncContext>
{
public:
	void SetUp() { UnitTestBase::SetUp(); }
	void TearDown() { UnitTestBase::TearDown(); }
};

TEST_F(TriggerEventUnitTest, NotTriggered)
{
	ASSERT_EQ(E_ILLEGAL_METHOD_CALL, e0.cancelTimer());
	e0.setDelayTimer(&mockState0, 100);
	ASSERT_EQ(TSM_S_TIMER_IS_STOPPED, e0.cancelTimer());
}

// Context timer.
TEST_F(TriggerEventUnitTest, 0)
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
TEST_F(TriggerEventUnitTest, 1)
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
TEST_F(TriggerEventUnitTest, 2)
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
	ASSERT_EQ(S_OK, mockState0.cancelEventTimer(&e0, CANCEL_TIMER_TIMEOUT));
	events = getPendingEvents(mockState0);
	EXPECT_EQ(0, events.size());
	EXPECT_TRUE(e0.deleted());
	Sleep(500);
}

// Cancel interval state timer
TEST_F(TriggerEventUnitTest, 3)
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
	ASSERT_EQ(S_OK, mockState0.cancelEventTimer(&e0, CANCEL_TIMER_TIMEOUT));
	events = getPendingEvents(mockState0);
	EXPECT_EQ(0, events.size());
	Sleep(10);
	EXPECT_TRUE(e0.deleted());
	Sleep(500);
}

// Cancel event timer of state on State::exit()
TEST_F(TriggerEventUnitTest, 4)
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

// Accuracy test
TEST_F(TriggerEventUnitTest, 5)
{
	static const DWORD expectedTimes[] = { 100, 200, 200, 200 };
	static const int TIME_COUNT = ARRAYSIZE(expectedTimes);
	ULONGLONG times[TIME_COUNT];
	int timeCount = 0;

	EXPECT_CALL(e0, preHandle(&mockContext)).Times(AnyNumber());
	EXPECT_CALL(e0, postHandle(&mockContext, S_OK)).Times(AnyNumber());
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, _))
		.WillRepeatedly(Invoke([&times, &timeCount](MockAsyncContext*, MockEvent_t* event, tsm::IState**)
		{
			if(timeCount < TIME_COUNT) {
				times[timeCount] = GetTickCount64();
				EXPECT_EQ(timeCount, event->_getTimeoutCount());
				timeCount++;
			} else {
				ADD_FAILURE() << "Time count overflow: count = " << timeCount;
			}
			return S_OK;
		}));

	auto startTime = GetTickCount64();
	e0.setTimer(&mockContext, 100, 200);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));
	Sleep(200 * TIME_COUNT);
	ASSERT_HRESULT_SUCCEEDED(mockContext.cancelEventTimer(&e0, CANCEL_TIMER_TIMEOUT));

	ASSERT_EQ(TIME_COUNT, timeCount);

	LOG4CPLUS_INFO(logger, "Start time=" << startTime);

	// Check that the Event has been triggered by
	// 50 mSec delay and 100 mSec interval with an accuracy of less than 16 mSec.
	for(int i = 0; i < TIME_COUNT; i++)
	{
		auto time = (DWORD)(times[i] - startTime);
		LOG4CPLUS_INFO(logger, "  " << time << ", diff=" << time - expectedTimes[i]);
		EXPECT_NEAR(time, expectedTimes[i], 16);
		startTime = times[i];
	}
}


class HandleEventUnitTest : public TimerUnitTest<MockAsyncContext>
{
public:
	void SetUp() { UnitTestBase::SetUp(); }
	void TearDown() { UnitTestBase::TearDown(); }
};

// Terminate timer thread by error of handleEvent() method.
TEST_F(HandleEventUnitTest, 0)
{
	auto hr = E_ABORT;
	EXPECT_CALL(e0, preHandle(&mockContext)).Times(1);
	EXPECT_CALL(e0, postHandle(&mockContext, hr))
		.WillOnce(Return(hr));
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(hr));

	e0.setTimer(&mockState0, 100, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.handleEvent(&e0));
	Sleep(50);
	auto events = getPendingEvents(mockState0);
	EXPECT_EQ(1, events.size());
	EXPECT_NE(events.end(), events.find(&e0));
	Sleep(100);
	events = getPendingEvents(mockState0);
	EXPECT_EQ(0, events.size());
	EXPECT_TRUE(e0.deleted());
}
