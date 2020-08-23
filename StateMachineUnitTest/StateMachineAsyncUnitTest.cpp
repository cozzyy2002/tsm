#include "stdafx.h"

#include "Mocks.h"
#include <vector>

using namespace testing;

class StateMachineAsyncUnitTest : public Test
{
public:
	using MockEvent_t = MockEvent<MockAsyncContext>;

	void SetUp() {
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, nullptr)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
		ASSERT_EQ(S_OK, mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, mockContext.shutdown());
		EXPECT_EQ(nullptr, mockContext._getCurrentState());
	}

	void createEvents(int count = 1) {
		for(int id = 0; id < count; id++) {
			mockEvents.push_back(new NiceMock<MockEvent_t>(id));
		}
	}

	MockAsyncContext mockContext;
	// Note: Use NiceMock to ommit `EXPECT_CALL(Event, preHandle())`.
	std::vector<CComPtr<NiceMock<MockEvent_t>>> mockEvents;
	MockState<MockAsyncContext> mockState0, mockState1;
};

// Identify event object and manage it's life time
TEST_F(StateMachineAsyncUnitTest, 0)
{
	NiceMock<MockEvent_t> e0, e1, e2;
	EXPECT_CALL(mockState0, handleEvent(&mockContext, _, Not(nullptr)))
		.WillOnce(Invoke([&](MockAsyncContext* context, MockEvent_t* event, tsm::IState**)
		{
			EXPECT_EQ(&e0, event);
			Sleep(10);
			return S_OK;
		}))
		.WillOnce(Invoke([&](MockAsyncContext* context, MockEvent_t* event, tsm::IState**)
		{
			EXPECT_EQ(&e1, event);
			return S_OK;
		}))
		.WillOnce(Invoke([&](MockAsyncContext* context, MockEvent_t* event, tsm::IState**)
		{
			EXPECT_EQ(&e2, event);
			return S_OK;
		}));

		ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));
		ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e1));
		ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e2));
		Sleep(100);

		EXPECT_TRUE(e0.deleted());
		EXPECT_TRUE(e1.deleted());
		EXPECT_TRUE(e2.deleted());
}

// Default event priority.
// State::handleEvent() should be called by order of AsyncContext::triggerEvent().
TEST_F(StateMachineAsyncUnitTest, 1)
{
	static const int EventsCount = 4;
	int actualEventCount = 0;
	EXPECT_CALL(mockState0, handleEvent(&mockContext, Not(nullptr), _))
		.WillRepeatedly(Invoke([&actualEventCount](MockAsyncContext* context, MockEvent_t* event, tsm::IState**)
		{
			// Wait for all triggerEvent() to be called.
			if(actualEventCount == 0) Sleep(10);
			EXPECT_EQ(actualEventCount++, event->id);
			return S_OK;
		}));

	createEvents(EventsCount);
	for(int i = 0; i < EventsCount; i++) {
		ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(mockEvents[i]));
		// Wait for 1st State::handleEvent() to be called.
		if(i == 0) Sleep(5);
	}

	Sleep(100);
	EXPECT_EQ(EventsCount, actualEventCount);
}

// Data for priority test.
using StateMachineAsyncPriorityUnitTestData = struct {
	int priorities[5];
	int sequence[5];
};

static const StateMachineAsyncPriorityUnitTestData stateMachineAsyncPriorityUnitTestData[] = {
	{ { 0,  0,  0,  0,  0 }, { 0, 1, 2, 3, 4 } },	// Same priority.
	{ { 0, -2,  0,  0,  0 }, { 0, 2, 3, 4, 1 } },	// First is lower.
	{ { 0,  0,  0,  0,  2 }, { 0, 4, 1, 2, 3 } },	// Last is Higher.
	{ { 0,  0, -2,  0,  0 }, { 0, 1, 3, 4, 2 } },	// Middle is lower.
	{ { 0,  0,  0,  2,  0 }, { 0, 3, 1, 2, 4 } },	// Middle is higher.
	{ { 0,  4,  3,  2,  1 }, { 0, 1, 2, 3, 4 } },	// Priority order.
	{ { 0,  1,  2,  3,  4 }, { 0, 4, 3, 2, 1 } },	// Reverse priority order.
};

class StateMachineAsyncPriorityUnitTest
	: public StateMachineAsyncUnitTest
	, public WithParamInterface<StateMachineAsyncPriorityUnitTestData>
{
public:
	void SetUp() {
		StateMachineAsyncUnitTest::SetUp();
	}
	void TearDown() {
		StateMachineAsyncUnitTest::TearDown();
	}
};

TEST_P(StateMachineAsyncPriorityUnitTest, sequence)
{
	auto& param = GetParam();

	int actualEventCount = 0;
	EXPECT_CALL(mockState0, handleEvent(&mockContext, Not(nullptr), _))
		.WillRepeatedly(Invoke([&actualEventCount, param](MockAsyncContext* context, MockEvent_t* event, tsm::IState**)
		{
			auto index = actualEventCount++;
			// Wait for all triggerEvent() to be called.
			if(index == 0) Sleep(100);

			if(index < ARRAYSIZE(param.sequence)) {
				EXPECT_EQ(param.sequence[index], event->id);
			} else {
				ADD_FAILURE() << "Unexpected event: count=" << actualEventCount << ", ID=" << event->id;
			}
			return S_OK;
		}));

	static const int eventCount = ARRAYSIZE(param.priorities);
	createEvents(eventCount);
	for(int i = 0; i < eventCount; i++) {
		mockEvents[i]->setPriority(param.priorities[i]);
		ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(mockEvents[i]));
		// Wait for 1st State::handleEvent() to be called.
		if(i == 0) Sleep(50);
	}

	Sleep(100);
	EXPECT_EQ(eventCount, actualEventCount);
}

INSTANTIATE_TEST_SUITE_P(priority, StateMachineAsyncPriorityUnitTest, ValuesIn(stateMachineAsyncPriorityUnitTestData));
