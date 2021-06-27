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

class AssertUnitTest : public ::UnitTest<MockContext>
{
public:
	class IAssert
	{
	public:
		virtual void proc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line) = 0;
		virtual void writer(LPCTSTR msg) = 0;
		virtual ~IAssert() {}
	};

	class MockAssert : public IAssert
	{
	public:
		MOCK_METHOD4(proc, void(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line));
		MOCK_METHOD1(writer, void(LPCTSTR msg));
	};

	void SetUp() {
		// Save default Assert functions.
		defaultProc = Assert::onAssertFailedProc;
		defaultWriter = Assert::onAssertFailedWriter;

		pMockAssert = new MockAssert();
	}
	void TearDown() {
		// Restore Assert functions.
		Assert::onAssertFailedProc = defaultProc;
		Assert::onAssertFailedWriter = defaultWriter;

		delete pMockAssert;
	}

	using Assert = tsm::Assert;
	Assert::OnAssertFailedProc defaultProc;
	Assert::OnAssertFailedWriter defaultWriter;

	// Static mock object for lambda to access this member without capture clause.
	static MockAssert* pMockAssert;
};

AssertUnitTest::MockAssert* AssertUnitTest::pMockAssert;

TEST_F(AssertUnitTest, proc)
{
	HRESULT hResult;
	EXPECT_CALL(*pMockAssert, proc(_, _, _, _)).WillOnce(
		[&hResult](HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
		{
			hResult = hr;
			_tprintf_s(_T("OnAssertFailedProc(0x%x)\n"), hr);
		}
	);
	EXPECT_CALL(*pMockAssert, writer(_)).Times(0);

	Assert::onAssertFailedProc = [](HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
	{
		AssertUnitTest::pMockAssert->proc(hr, exp, sourceFile, line);
	};

	auto hr = mockContext.handleEvent(&mockEvent);
	_tprintf_s(_T("HResult returned = 0x%08x\n"), hr);
	ASSERT_EQ(hr, hResult);
}

TEST_F(AssertUnitTest, writer)
{
	std::tstring assertMessage;
	EXPECT_CALL(*pMockAssert, proc(_, _, _, _)).Times(0);
	EXPECT_CALL(*pMockAssert, writer(_)).WillOnce(([&assertMessage](LPCTSTR msg)
	{
		assertMessage = msg;
		_tprintf_s(_T("%s\n"), msg);
	}));

	Assert::onAssertFailedProc = nullptr;
	Assert::onAssertFailedWriter = [](LPCTSTR msg)
	{
		AssertUnitTest::pMockAssert->writer(msg);
	};

	auto hr = mockContext.handleEvent(&mockEvent);
	_tprintf_s(_T("HResult returned = 0x%08x\n"), hr);

	TCHAR hrStr[10];
	_stprintf_s(hrStr, _T("%x"), hr);
	ASSERT_NE(assertMessage.find(hrStr), std::tstring::npos);
}


// -------- UnknownImplUnitTest --------

class TesteeBase
{
public:
	TesteeBase() { objectCount++; }
	virtual ~TesteeBase() { objectCount--; }
	static int objectCount;
};

int TesteeBase::objectCount = 0;

// Testee class derived from IUnknown.
class TesteeIUnknown : public IUnknown, public TesteeBase
{
public:
	TesteeIUnknown() : impl(this) {}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override {
		return impl.QueryInterface(riid, ppvObject);
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return impl.AddRef(); }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return impl.Release(); }

	tsm::UnknownImpl<TesteeIUnknown> impl;
};

// Testee class derived from IUnknown and extra interface.
class TesteeISupportErrorInfo : public ISupportErrorInfo, public TesteeBase
{
public:
	TesteeISupportErrorInfo() : impl(this) {}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
	{
		static const QITAB qitab[] = {
			QITABENT(TesteeISupportErrorInfo, ISupportErrorInfo),
			{ 0 }
		};
		return impl.QueryInterface(riid, ppvObject, qitab);
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return impl.AddRef(); }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return impl.Release(); }

	virtual HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo(REFIID riid) { return E_NOTIMPL; }

	tsm::UnknownImpl<TesteeISupportErrorInfo> impl;
};

using UnknownImplUnitTestTesteeTypes = Types<TesteeIUnknown, TesteeISupportErrorInfo>;

template<class T>
class UnknownImplUnitTest : public Test
{
public:
	using Testee = T;
	void SetUp() {
		Testee::objectCount = 0;
	}
};

TYPED_TEST_SUITE(UnknownImplUnitTest, UnknownImplUnitTestTesteeTypes);

TYPED_TEST(UnknownImplUnitTest, create_delete)
{
	{
		CComPtr<Testee> testee;
		testee = new Testee();
		ASSERT_EQ(Testee::objectCount, 1);
		ASSERT_EQ(testee->impl.getCRef(), 1);
	}
	ASSERT_EQ(Testee::objectCount, 0);
}

TYPED_TEST(UnknownImplUnitTest, cRef)
{
	CComPtr<Testee> testee1(new Testee());
	CComPtr<Testee> testee2(testee1.p);

	ASSERT_EQ(testee1->impl.getCRef(), 2);
	ASSERT_EQ(Testee::objectCount, 1);
	testee1.Release();
	ASSERT_EQ(testee2->impl.getCRef(), 1);
	ASSERT_EQ(Testee::objectCount, 1);
	testee2.Release();
	ASSERT_EQ(Testee::objectCount, 0);
}


TYPED_TEST(UnknownImplUnitTest, QueryInterface)
{
	{
		CComPtr<Testee> testee(new Testee());
		CComPtr<IUnknown> unk;

		ASSERT_HRESULT_SUCCEEDED(testee.QueryInterface(&unk));
		ASSERT_EQ(testee, unk);
		ASSERT_EQ(testee->impl.getCRef(), 2);

		// Interface that Testee class does not implement.
		CComPtr<IRegistrar> reg;
		ASSERT_EQ(testee.QueryInterface(&reg), E_NOINTERFACE);

		ASSERT_EQ(testee.QueryInterface((IRegistrar**)nullptr), E_POINTER);
	}

	// All objects should be deleted.
	ASSERT_EQ(Testee::objectCount, 0);
}

TEST(TesteeISupportErrorInfoUnitTest, QueryInterface)
{
	TesteeISupportErrorInfo::objectCount = 0;
	{
		CComPtr<TesteeISupportErrorInfo> testee(new TesteeISupportErrorInfo());
		ASSERT_EQ(testee->impl.getCRef(), 1);
		ASSERT_EQ(TesteeISupportErrorInfo::objectCount, 1);

		CComPtr<ISupportErrorInfo> sup;
		ASSERT_HRESULT_SUCCEEDED(testee.QueryInterface(&sup));
		ASSERT_EQ(testee->impl.getCRef(), 2);
		ASSERT_EQ(TesteeISupportErrorInfo::objectCount, 1);
	}

	// All objects should be deleted.
	ASSERT_EQ(TesteeISupportErrorInfo::objectCount, 0);
}
