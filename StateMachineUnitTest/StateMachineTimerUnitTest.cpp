#include "stdafx.h"

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

// Context::triggerDelayedEvent()
TEST_F(StateMachineTriggerEventUnitTest, 0)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerDelayedEvent(&e0, 100));

	ASSERT_EQ(1, mockContext.m_timers.size());
	Sleep(110);
	ASSERT_EQ(0, mockContext.m_timers.size());
	EXPECT_TRUE(e0.deleted());
}

// State::triggerDelayedEvent()
TEST_F(StateMachineTriggerEventUnitTest, 1)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(mockState0.triggerDelayedEvent(&mockContext, &e0, 100));

	ASSERT_EQ(1, mockState0.m_timers.size());
	Sleep(110);
	ASSERT_EQ(0, mockState0.m_timers.size());
	EXPECT_TRUE(e0.deleted());
}
