// StateMachineUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Mocks.h"

using namespace testing;

template<class T>
static LPCSTR getObjectName(T* obj) { return typeid(*obj).name(); }

class StateMachineUnitTest : public Test
{
public:
	~StateMachineUnitTest() {
		// Prevent Unknown::Release() method of MockState from being called after deleting.
		mockContext.m_currentState.Release();
	}

	MockAsyncContext mockContext;
	MockEvent mockEvent;
	MockAsyncState mockState0, mockState1;
};

// -------------------------
class StateMachineSetupUnitTest : public StateMachineUnitTest
{
public:
	void TearDown() {
		EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());
	}
};

// StateMachine::setup(Event* = nullptr)
TEST_F(StateMachineSetupUnitTest, 0)
{
	EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(mockContext.setup(&mockState0));
	ASSERT_HRESULT_SUCCEEDED(mockContext.waitReady());

	EXPECT_EQ(&mockState0, mockContext.m_currentState);
	EXPECT_FALSE(mockState0.deleted());
	EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// StateMachine::setup(Event* = event)
TEST_F(StateMachineSetupUnitTest, 1)
{
	EXPECT_CALL(mockState0, entry(&mockContext, &mockEvent, _)).WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(mockContext.setup(&mockState0, &mockEvent));
	ASSERT_HRESULT_SUCCEEDED(mockContext.waitReady());

	EXPECT_EQ(&mockState0, mockContext.m_currentState);
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
	EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// State::entry() returns error
TEST_F(StateMachineSetupUnitTest, 2)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockState0, entry(&mockContext, &mockEvent, _)).WillOnce(Return(hr));

	ASSERT_EQ(hr, mockContext.setup(&mockState0, &mockEvent));
	ASSERT_HRESULT_SUCCEEDED(mockContext.waitReady());

	EXPECT_EQ(&mockState0, mockContext.m_currentState);
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
	EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// StateMachine::setup() was called twice.
// 2nd call should fail.
TEST_F(StateMachineSetupUnitTest, 3)
{
	EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(mockContext.setup(&mockState0));
	ASSERT_EQ(E_ILLEGAL_METHOD_CALL, mockContext.setup(&mockState0, &mockEvent));
	ASSERT_HRESULT_SUCCEEDED(mockContext.waitReady());

	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_EQ(1, mockState0.getReferenceCount());
	EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// StateMachine::shutdown() is called before setup().
TEST_F(StateMachineSetupUnitTest, 4)
{
	// shutdown() can be called even if before setup().
	ASSERT_HRESULT_SUCCEEDED(mockContext.shutdown());

	EXPECT_EQ(nullptr, mockContext.m_currentState);
}

// StateMachine::handleEvent() is called before setup().
TEST_F(StateMachineSetupUnitTest, 5)
{
	ASSERT_EQ(E_ILLEGAL_METHOD_CALL, mockContext.handleEvent(&mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
}

// -------------------------
class StateMachineEventUnitTest : public StateMachineUnitTest
{
public:
	void SetUp() {
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));
		ASSERT_HRESULT_SUCCEEDED(mockContext.setup(&mockState0));
	}
	void TearDown() {
		ASSERT_HRESULT_SUCCEEDED(mockContext.shutdown());
	}
};

// No state transition occurs.
TEST_F(StateMachineEventUnitTest, 0)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState0, mockContext.m_currentState);
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

	ASSERT_HRESULT_SUCCEEDED(mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState1, mockContext.m_currentState);
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_TRUE(mockState0.deleted());
	EXPECT_FALSE(mockState1.deleted());
}

// State::handleEvent() returns error.
TEST_F(StateMachineEventUnitTest, 2)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(Return(hr));
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(hr, mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState0, mockContext.m_currentState);
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}
