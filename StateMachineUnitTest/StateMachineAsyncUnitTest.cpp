#include "stdafx.h"

#include "Mocks.h"
#include <vector>

using namespace testing;

class StateMachineAsyncUnitTest : public Test
{
public:
	void SetUp() {
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
		ASSERT_EQ(S_OK, mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, mockContext.shutdown());
		EXPECT_EQ(nullptr, mockContext._getCurrentState());
	}

	void createEvents(int count = 1) {
		for(int id = 0; id < count; id++) {
			mockEvents.push_back(new MockEvent(id));
		}
	}

	MockAsyncContext mockContext;
	std::vector<CComPtr<MockEvent>> mockEvents;
	MockState<MockAsyncContext> mockState0, mockState1;
};

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
		.WillRepeatedly(Invoke([&actualEventCount, param](MockAsyncContext* context, MockEvent* event, tsm::IState**)
		{
			auto index = actualEventCount++;
			// Wait for all triggerEvent() to be called.
			if(index == 0) Sleep(10);

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
		ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(mockEvents[i], param.priorities[i]));
		// Wait for 1st State::handleEvent() to be called.
		if(i == 0) Sleep(5);
	}

	Sleep(100);
	EXPECT_EQ(eventCount, actualEventCount);
}

INSTANTIATE_TEST_CASE_P(priority, StateMachineAsyncPriorityUnitTest, ValuesIn(stateMachineAsyncPriorityUnitTestData));
