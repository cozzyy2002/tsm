// UnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Mocks.h"
#include <StateMachine/StateMachineMessage.h>

using namespace testing;

TEST(ErrorCodeUnitTest, Success)
{
	ASSERT_EQ(S_OK, TSM_S_OK);
	ASSERT_EQ(S_FALSE, TSM_S_DONE);
}

template<class T>
static LPCSTR getObjectName(T* obj) { return typeid(*obj).name(); }

typedef Types<MockContext, MockAsyncContext> ContextTypes;

template<class C>
class UnitTest : public Test
{
public:
	using MockEvent_t = MockEvent<C>;

	void SetUp() {}
	void TearDown() {
		EXPECT_EQ(nullptr, mockContext._getCurrentState());
	}

	C mockContext;
	MockEvent_t mockEvent;
	MockState<C> mockState0, mockState1, mockState2, mockState3;
};

// -------------------------
template<class C>
class SetupUnitTest : public ::UnitTest<C>
{
public:
	void SetUp() {
		UnitTest::SetUp();
	}
	void TearDown() {
		EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());

		UnitTest::TearDown();
	}
};

TYPED_TEST_SUITE(SetupUnitTest, ContextTypes);

// StateMachine::setup(Event* = nullptr)
TYPED_TEST(SetupUnitTest, 0)
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
TYPED_TEST(SetupUnitTest, 1)
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
TYPED_TEST(SetupUnitTest, 2)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockState0, entry(&mockContext, &mockEvent, _)).WillOnce(Return(hr));

	if(mockContext.isAsync()) {
		// AsyncContext::getAsyncExitCode() should return the error code from State::entry().
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0, &mockEvent));
		ASSERT_EQ(TSM_S_NO_WORKER_THREAD, mockContext.waitReady());
		HRESULT hrExitCode;
		ASSERT_EQ(S_OK, mockContext.getAsyncExitCode(&hrExitCode));
		ASSERT_EQ(hr, hrExitCode);
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
TYPED_TEST(SetupUnitTest, 3)
{
	EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));

	ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
	ASSERT_EQ(TSM_E_SETUP_HAS_BEEN_MADE, mockContext.setup(&mockState0, &mockEvent));
	ASSERT_EQ(S_OK, mockContext.waitReady());

	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_EQ(1, mockState0.getReferenceCount());
	ASSERT_EQ(S_OK, mockContext.shutdown());
	EXPECT_TRUE(mockState0.deleted());
}

// StateMachine::shutdown() is called before setup().
TYPED_TEST(SetupUnitTest, 4)
{
	// shutdown() can be called even if before setup().
	ASSERT_EQ(S_OK, mockContext.shutdown());

	EXPECT_EQ(nullptr, mockContext._getCurrentState());
}

// StateMachine::handleEvent() is called before setup().
TYPED_TEST(SetupUnitTest, 5)
{
	ASSERT_EQ(TSM_E_SETUP_HAS_NOT_BEEN_MADE, mockContext.handleEvent(&mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
}

// -------------------------
template<class C>
class EventUnitTest : public ::UnitTest<C>
{
public:
	void SetUp() {
		UnitTest::SetUp();

		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
		ASSERT_EQ(S_OK, mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, mockContext.shutdown());

		UnitTest::TearDown();
	}
};

TYPED_TEST_SUITE(EventUnitTest, ContextTypes);

// No state transition occurs.
// Event::preHandle() returns S_OK.
TYPED_TEST(EventUnitTest, NoStateTransition)
{
	EXPECT_CALL(mockEvent, preHandle(&mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockEvent, postHandle(&mockContext, S_OK))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(S_OK, mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

// Event::preHandle() returns S_FALSE(State::handleEvent() is not called).
TYPED_TEST(EventUnitTest, PreHandleFalse)
{
	EXPECT_CALL(mockEvent, preHandle(&mockContext))
		.WillOnce(Return(S_FALSE));
	EXPECT_CALL(mockEvent, postHandle(_, _)).Times(0);
	EXPECT_CALL(mockState0, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(S_FALSE, mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

// Event::preHandle() returns error(State::handleEvent() is not called).
TYPED_TEST(EventUnitTest, PreHandleError)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockEvent, preHandle(&mockContext))
		.WillOnce(Return(hr));
	EXPECT_CALL(mockEvent, postHandle(_, _)).Times(0);
	EXPECT_CALL(mockState0, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(hr, mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

// State transition occurs.
TYPED_TEST(EventUnitTest, StateTransition)
{
	EXPECT_CALL(mockEvent, preHandle(&mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockEvent, postHandle(&mockContext, S_OK))
		.WillOnce(Return(S_OK));
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
TYPED_TEST(EventUnitTest, HandleEventError)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockEvent, preHandle(&mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockEvent, postHandle(&mockContext, hr))
		.WillOnce(Return(hr));
	EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, Not(nullptr)))
		.WillOnce(Return(hr));
	EXPECT_CALL(mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(hr, mockContext.handleEvent(&mockEvent));

	EXPECT_EQ(&mockState0, mockContext._getCurrentState());
	EXPECT_TRUE(mockEvent.deleted());
	EXPECT_FALSE(mockState0.deleted());
}

// State::exit() returns error.
TYPED_TEST(EventUnitTest, ExitError)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockEvent, preHandle(&mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockEvent, postHandle(&mockContext, hr))
		.WillOnce(Return(hr));
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
TYPED_TEST(EventUnitTest, EntryError)
{
	auto hr = E_ABORT;
	EXPECT_CALL(mockEvent, preHandle(&mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(mockEvent, postHandle(&mockContext, hr))
		.WillOnce(Return(hr));
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
class SubStateUnitTest : public ::UnitTest<C>
{
public:
	void SetUp() {
		UnitTest::SetUp();
	}
	void TearDown() {
		EXPECT_HRESULT_SUCCEEDED(mockContext.shutdown());

		UnitTest::TearDown();
	}
};

TYPED_TEST_SUITE(SubStateUnitTest, ContextTypes);

// State chain: State0 -> State1
// State1 returns State0 as next state.
TYPED_TEST(SubStateUnitTest, 0)
{
	{
		InSequence _sequence;
		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));
		EXPECT_CALL(mockEvent, preHandle(&mockContext))
			.WillOnce(Return(S_OK));
		EXPECT_CALL(mockState0, handleEvent(&mockContext, &mockEvent, _))
			.WillOnce(DoAll(SetArgPointee<2>(&mockState1), Return(S_OK)));
		EXPECT_CALL(mockState1, entry(&mockContext, &mockEvent, &mockState0)).WillOnce(Return(S_OK));
		EXPECT_CALL(mockEvent, postHandle(&mockContext, S_OK))
			.WillOnce(Return(S_OK));
		EXPECT_CALL(mockEvent, preHandle(&mockContext))
			.WillOnce(Return(S_OK));
		EXPECT_CALL(mockState1, handleEvent(&mockContext, &mockEvent, _))
			.WillOnce(DoAll(SetArgPointee<2>(&mockState0), Return(S_OK)));
		EXPECT_CALL(mockState1, exit(&mockContext, &mockEvent, &mockState0)).WillOnce(Return(S_OK));
		EXPECT_CALL(mockEvent, postHandle(&mockContext, S_OK))
			.WillOnce(Return(S_OK));
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
class NotImplTest : public ::UnitTest<MockContext>
{
public:
	void SetUp() {
		UnitTest::SetUp();

		EXPECT_CALL(mockState0, entry(&mockContext, nullptr, _)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, mockContext.setup(&mockState0));
		ASSERT_EQ(S_OK, mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, mockContext.shutdown());

		UnitTest::TearDown();
	}
};

TEST_F(NotImplTest, 0)
{
	HRESULT hrExitCode;
	EXPECT_EQ(E_NOTIMPL, mockContext.getAsyncExitCode(&hrExitCode));

	MockEvent<MockContext> mockEvent;
	EXPECT_EQ(TSM_E_NOT_SUPPORTED, mockContext.triggerEvent(&mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
}
