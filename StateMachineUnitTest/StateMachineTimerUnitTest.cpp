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

// Context timer.
TEST_F(StateMachineTriggerEventUnitTest, 0)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(S_OK));

	e0.setTimer(&mockContext, 50);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));

	//ASSERT_EQ(1, mockContext.m_timers.size());
	Sleep(110);
	ASSERT_EQ(0, mockContext.m_timers.size());
	EXPECT_TRUE(e0.deleted());
}

// State timer
TEST_F(StateMachineTriggerEventUnitTest, 1)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(Return(S_OK));

	e0.setTimer(&mockState0, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));

	ASSERT_EQ(1, mockState0.m_timers.size());
	Sleep(110);
	ASSERT_EQ(0, mockState0.m_timers.size());
	EXPECT_TRUE(e0.deleted());
}

// Cancel event timer of state on State::exit()
TEST_F(StateMachineTriggerEventUnitTest, 2)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &e0, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
	// triggerDelyedEvent(&e1) should be canceled.
	EXPECT_CALL(mockState0, handleEvent(_, &e1, _)).Times(0);
	EXPECT_CALL(mockState0, exit(&mockContext, &e0, &mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState1, entry(&mockContext, &e0, &mockState0)).WillOnce(Return(S_OK));

	// Start delayed event e1 to be canceled.
	e1.setTimer(&mockState0, 100);
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e1));
	Sleep(10);
	ASSERT_EQ(1, mockState0.m_timers.size());
	// mockState0 -> mockState1 -> Cancel delayed event e1 of mockState0.
	ASSERT_HRESULT_SUCCEEDED(mockContext.triggerEvent(&e0));
	Sleep(100);
}
