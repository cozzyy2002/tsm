// StateMachineUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Mocks.h"

using namespace testing;

template<class T>
static LPCSTR getObjectName(T* obj) { return typeid(*obj).name(); }

typedef Types<MockContext, MockAsyncContext> ContextTypes;

template<class C>
class StateMachineUnitTest : public Test
{
public:
	void SetUp() {}
	void TearDown() {
		EXPECT_EQ(nullptr, mockContext._getCurrentState());
	}

	C mockContext;
	MockEvent mockEvent;
	MockState<C> mockState0, mockState1, mockState2, mockState3;
};

// -------------------------
template<class C>
class StateMachineSetupUnitTest : public StateMachineUnitTest<C>
{
public:
	void SetUp() {
		StateMachineUnitTest::SetUp();
	}
	void TearDown() {
		EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());

		StateMachineUnitTest::TearDown();
	}
};

TYPED_TEST_CASE(StateMachineSetupUnitTest, ContextTypes);

// StateMachine::setup(Event* = nullptr)
TYPED_TEST(StateMachineSetupUnitTest, 0)
{
	EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));

	ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
	ASSERT_EQ(S_OK, mockContext.waitReady());

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_FALSE(mockState0.deleted());
	ASSERT_EQ(S_OK, mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// StateMachine::setup(Event* = event)
TYPED_TEST(StateMachineSetupUnitTest, 1)
{
	EXPECT_CALL(mockState0, entry(&mockContext, &mockEvent, _)).WillOnce(Return(S_OK));

	ASSERT_EQ(S_OK, mockContext.setup(&mockState0, &mockEvent));
	ASSERT_EQ(S_OK, mockContext.waitReady());

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
	ASSERT_EQ(S_OK, mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// State::entry() returns error
TYPED_TEST(StateMachineSetupUnitTest, 2)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockState0, entry(&mockContext, &mockEvent, _)).WillOnce(Return(hr));

	if(mockContext.isAsync()) {
		// AsyncContext::waitRady() should return the error code from State::entry().
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0, &mockEvent));
		ASSERT_EQ(hr, mockContext.waitReady());
	} else {
		// Context::setup() should return the error code from State::entry().
		ASSERT_EQ(hr, mockContext.setup(&mockState0, &mockEvent));
	}

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
	ASSERT_EQ(S_OK, mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// StateMachine::setup() was called twice.
// 2nd call should fail.
TYPED_TEST(StateMachineSetupUnitTest, 3)
{
	EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));

	ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
	ASSERT_EQ(E_ILLEGAL_METHOD_CALL, mockContext.setup(&mockState0, &mockEvent));
	ASSERT_EQ(S_OK, mockContext.waitReady());

	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_EQ(1, mockState0.getReferenceCount());
	ASSERT_EQ(S_OK, mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// StateMachine::shutdown() is called before setup().
TYPED_TEST(StateMachineSetupUnitTest, 4)
{
	// shutdown() can be called even if before setup().
	ASSERT_EQ(S_OK, mockContext.shutdown());

	EXPECT_EQ(nullptr, mockContext._getCurrentState());
}

// StateMachine::handleEvent() is called before setup().
TYPED_TEST(StateMachineSetupUnitTest, 5)
{
	ASSERT_EQ(E_ILLEGAL_METHOD_CALL, mockContext.handleEvent(&mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
}

// -------------------------
template<class C>
class StateMachineEventUnitTest : public StateMachineUnitTest<C>
{
public:
	void SetUp() {
		StateMachineUnitTest::SetUp();

		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
		ASSERT_EQ(S_OK, mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, mockContext.shutdown());

		StateMachineUnitTest::TearDown();
	}
};

TYPED_TEST_CASE(StateMachineEventUnitTest, ContextTypes);

// No state transition occurs.
TYPED_TEST(StateMachineEventUnitTest, 0)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

// State transition occurs.
TYPED_TEST(StateMachineEventUnitTest, 1)
{
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
	EXPECT_CALL(mockState0, exit(&mockContext, &mockEvent, &mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState1, entry(&mockContext, &mockEvent, &mockState0)).WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState1, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_TRUE(mockState0.deleted());
	EXPECT_FALSE(mockState1.deleted());
}

// State::handleEvent() returns error.
TYPED_TEST(StateMachineEventUnitTest, 2)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(Return(hr));
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(hr, mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

// State::exit() returns error.
TYPED_TEST(StateMachineEventUnitTest, 3)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
	EXPECT_CALL(mockState0, exit(&mockContext, &mockEvent, &mockState1)).WillOnce(Return(hr));
	EXPECT_CALL(mockState1, entry(&mockContext, &mockEvent, &mockState0)).Times(0);

	ASSERT_EQ(hr, mockContext.handleEvent(&mockEvent));

	// mockState0 remains as current state.
	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
	EXPECT_TRUE(mockState1.deleted());
}

// State::entry() returns error.
TYPED_TEST(StateMachineEventUnitTest, 4)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
	EXPECT_CALL(mockState0, exit(&mockContext, &mockEvent, &mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(mockState1, entry(&mockContext, &mockEvent, &mockState0)).WillOnce(Return(hr));

	ASSERT_EQ(hr, mockContext.handleEvent(&mockEvent));

	// mockState1 becomes current state.
	EXPECT_EQ(&mockState1, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_TRUE(mockState0.deleted());
	EXPECT_FALSE(mockState1.deleted());
}

// -------------------------
template<class C>
class StateMachineSubStateUnitTest : public StateMachineUnitTest<C>
{
public:
	void SetUp() {
		StateMachineUnitTest::SetUp();
	}
	void TearDown() {
		EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());

		StateMachineUnitTest::TearDown();
	}
};

TYPED_TEST_CASE(StateMachineSubStateUnitTest, ContextTypes);

// State chain: State0 -> State1
// State1 returns State0 as next state.
TYPED_TEST(StateMachineSubStateUnitTest, 0)
{
	{
		InSequence _sequence;
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));
		EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, _))
			.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
		EXPECT_CALL(mockState1, entry(&mockContext, &mockEvent, &mockState0)).WillOnce(Return(S_OK));
		EXPECT_CALL(mockState1, handleEvent(&mockContext, &mockEvent, _))
			.WillOnce(DoAll(SetArgPointee<2>(&mockState0), Return(S_OK)));
		EXPECT_CALL(mockState1, exit(&mockContext, &mockEvent, &mockState0)).WillOnce(Return(S_OK));
	}
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
	ASSERT_EQ(S_OK, mockContext.waitReady());

	mockState1.setMasterState(&mockState0);
	ASSERT_EQ(S_OK, mockContext.handleEvent(&mockEvent));	// State0 -> State1
	ASSERT_EQ(S_OK, mockContext.handleEvent(&mockEvent));	// State1 -> State0(Sub state goes back to master state)

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_FALSE(mockState0.deleted());
	EXPECT_TRUE(mockState1.deleted());
	ASSERT_EQ(S_OK, mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// Test for Context to ensure that async operations are denied.
class StateMachineNotImplTest : public StateMachineUnitTest<MockContext>
{
public:
	void SetUp() {
		StateMachineUnitTest::SetUp();

		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
		ASSERT_EQ(S_OK, mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, mockContext.shutdown());

		StateMachineUnitTest::TearDown();
	}
};

TEST_F(StateMachineNotImplTest, 0)
{
	ASSERT_EQ(E_NOTIMPL, mockContext.triggerEvent(&mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
}
