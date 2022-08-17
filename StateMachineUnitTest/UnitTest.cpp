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
class UnitTestBase : public Test
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
class SetupUnitTest : public UnitTestBase<C>
{
public:
	void SetUp() {
		UnitTestBase<C>::SetUp();
	}
	void TearDown() {
		EXPECT_HRESULT_SUCCEEDED(this->mockContext.shutdown());

		UnitTestBase<C>::TearDown();
	}
};

TYPED_TEST_SUITE(SetupUnitTest, ContextTypes);

// StateMachine::setup(Event* = nullptr)
TYPED_TEST(SetupUnitTest, 0)
{
	EXPECT_CALL(this->mockState0, entry(&this->mockContext, nullptr, _)).WillOnce(Return(S_OK));

	ASSERT_EQ(S_OK, this->mockContext.setup(&this->mockState0));
	ASSERT_EQ(S_OK, this->mockContext.waitReady());

	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_FALSE(this->mockState0.deleted());
	ASSERT_EQ(S_OK, this->mockContext.shutdown());
	EXPECT_TRUE(this->mockState0.deleted());
}

// StateMachine::setup(Event* = event)
TYPED_TEST(SetupUnitTest, 1)
{
	EXPECT_CALL(this->mockState0, entry(&this->mockContext, &this->mockEvent, _)).WillOnce(Return(S_OK));

	ASSERT_EQ(S_OK, this->mockContext.setup(&this->mockState0, &this->mockEvent));
	ASSERT_EQ(S_OK, this->mockContext.waitReady());

	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_FALSE(this->mockState0.deleted());
	ASSERT_EQ(S_OK, this->mockContext.shutdown());
	EXPECT_TRUE(this->mockState0.deleted());
}

// State::entry() returns error
TYPED_TEST(SetupUnitTest, 2)
{
	auto hr = E_ABORT;
	EXPECT_CALL(this->mockState0, entry(&this->mockContext, &this->mockEvent, _)).WillOnce(Return(hr));

	if(this->mockContext.isAsync()) {
		// AsyncContext::getAsyncExitCode() should return the error code from State::entry().
		ASSERT_EQ(S_OK, this->mockContext.setup(&this->mockState0, &this->mockEvent));
		ASSERT_EQ(TSM_S_NO_WORKER_THREAD, this->mockContext.waitReady());
		HRESULT hrExitCode;
		ASSERT_EQ(S_OK, this->mockContext.getAsyncExitCode(&hrExitCode));
		ASSERT_EQ(hr, hrExitCode);
	} else {
		// Context::setup() should return the error code from State::entry().
		ASSERT_EQ(hr, this->mockContext.setup(&this->mockState0, &this->mockEvent));
	}

	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_FALSE(this->mockState0.deleted());
	ASSERT_EQ(S_OK, this->mockContext.shutdown());
	EXPECT_TRUE(this->mockState0.deleted());
}

// StateMachine::setup() was called twice.
// 2nd call should fail.
TYPED_TEST(SetupUnitTest, 3)
{
	EXPECT_CALL(this->mockState0, entry(&this->mockContext, nullptr, _)).WillOnce(Return(S_OK));

	ASSERT_EQ(S_OK, this->mockContext.setup(&this->mockState0));
	ASSERT_EQ(TSM_E_SETUP_HAS_BEEN_MADE, this->mockContext.setup(&this->mockState0, &this->mockEvent));
	ASSERT_EQ(S_OK, this->mockContext.waitReady());

	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_EQ(1, this->mockState0.getReferenceCount());
	ASSERT_EQ(S_OK, this->mockContext.shutdown());
	EXPECT_TRUE(this->mockState0.deleted());
}

// StateMachine::shutdown() is called before setup().
TYPED_TEST(SetupUnitTest, 4)
{
	// shutdown() can be called even if before setup().
	ASSERT_EQ(S_OK, this->mockContext.shutdown());

	EXPECT_EQ(nullptr, this->mockContext._getCurrentState());
}

// StateMachine::handleEvent() is called before setup().
TYPED_TEST(SetupUnitTest, 5)
{
	ASSERT_EQ(TSM_E_SETUP_HAS_NOT_BEEN_MADE, this->mockContext.handleEvent(&this->mockEvent));

	EXPECT_TRUE(this->mockEvent.deleted());
}

// -------------------------
template<class C>
class EventUnitTest : public UnitTestBase<C>
{
public:
	void SetUp() {
		UnitTestBase<C>::SetUp();

		EXPECT_CALL(this->mockState0, entry(&this->mockContext, nullptr, _)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, this->mockContext.setup(&this->mockState0));
		ASSERT_EQ(S_OK, this->mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, this->mockContext.shutdown());

		UnitTestBase<C>::TearDown();
	}
};

TYPED_TEST_SUITE(EventUnitTest, ContextTypes);

// No state transition occurs.
// Event::preHandle() returns S_OK.
TYPED_TEST(EventUnitTest, NoStateTransition)
{
	EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockEvent, postHandle(&this->mockContext, S_OK))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockState0, handleEvent(&this->mockContext, &this->mockEvent, Not(nullptr)))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(S_OK, this->mockContext.handleEvent(&this->mockEvent));

	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_FALSE(this->mockState0.deleted());
}

// Event::preHandle() returns S_FALSE(State::handleEvent() is not called).
TYPED_TEST(EventUnitTest, PreHandleFalse)
{
	EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
		.WillOnce(Return(S_FALSE));
	EXPECT_CALL(this->mockEvent, postHandle(_, _)).Times(0);
	EXPECT_CALL(this->mockState0, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(this->mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(S_FALSE, this->mockContext.handleEvent(&this->mockEvent));

	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_FALSE(this->mockState0.deleted());
}

// Event::preHandle() returns error(State::handleEvent() is not called).
TYPED_TEST(EventUnitTest, PreHandleError)
{
	auto hr = E_ABORT;
	EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
		.WillOnce(Return(hr));
	EXPECT_CALL(this->mockEvent, postHandle(_, _)).Times(0);
	EXPECT_CALL(this->mockState0, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(this->mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(hr, this->mockContext.handleEvent(&this->mockEvent));

	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_FALSE(this->mockState0.deleted());
}

// State transition occurs.
TYPED_TEST(EventUnitTest, StateTransition)
{
	EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockEvent, postHandle(&this->mockContext, S_OK))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockState0, handleEvent(&this->mockContext, &this->mockEvent, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&this->mockState1), Return(S_OK)));
	EXPECT_CALL(this->mockState0, exit(&this->mockContext, &this->mockEvent, &this->mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockState1, entry(&this->mockContext, &this->mockEvent, &this->mockState0)).WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(this->mockContext.handleEvent(&this->mockEvent));

	EXPECT_EQ(&this->mockState1, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_TRUE(this->mockState0.deleted());
	EXPECT_FALSE(this->mockState1.deleted());
}

// State::handleEvent() returns error.
TYPED_TEST(EventUnitTest, HandleEventError)
{
	auto hr = E_ABORT;
	EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockEvent, postHandle(&this->mockContext, hr))
		.WillOnce(Return(hr));
	EXPECT_CALL(this->mockState0, handleEvent(&this->mockContext, &this->mockEvent, Not(nullptr)))
		.WillOnce(Return(hr));
	EXPECT_CALL(this->mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(hr, this->mockContext.handleEvent(&this->mockEvent));

	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_FALSE(this->mockState0.deleted());
}

// State::exit() returns error.
TYPED_TEST(EventUnitTest, ExitError)
{
	auto hr = E_ABORT;
	EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockEvent, postHandle(&this->mockContext, hr))
		.WillOnce(Return(hr));
	EXPECT_CALL(this->mockState0, handleEvent(&this->mockContext, &this->mockEvent, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&this->mockState1), Return(S_OK)));
	EXPECT_CALL(this->mockState0, exit(&this->mockContext, &this->mockEvent, &this->mockState1)).WillOnce(Return(hr));
	EXPECT_CALL(this->mockState1, entry(&this->mockContext, &this->mockEvent, &this->mockState0)).Times(0);

	ASSERT_EQ(hr, this->mockContext.handleEvent(&this->mockEvent));

	// mockState0 remains as current state.
	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_FALSE(this->mockState0.deleted());
	EXPECT_TRUE(this->mockState1.deleted());
}

// State::entry() returns error.
TYPED_TEST(EventUnitTest, EntryError)
{
	auto hr = E_ABORT;
	EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockEvent, postHandle(&this->mockContext, hr))
		.WillOnce(Return(hr));
	EXPECT_CALL(this->mockState0, handleEvent(&this->mockContext, &this->mockEvent, Not(nullptr)))
		.WillOnce(DoAll(SetArgPointee<2>(&this->mockState1), Return(S_OK)));
	EXPECT_CALL(this->mockState0, exit(&this->mockContext, &this->mockEvent, &this->mockState1)).WillOnce(Return(S_OK));
	EXPECT_CALL(this->mockState1, entry(&this->mockContext, &this->mockEvent, &this->mockState0)).WillOnce(Return(hr));

	ASSERT_EQ(hr, this->mockContext.handleEvent(&this->mockEvent));

	// mockState1 becomes current state.
	EXPECT_EQ(&this->mockState1, this->mockContext._getCurrentState());
	EXPECT_TRUE(this->mockEvent.deleted());
	EXPECT_TRUE(this->mockState0.deleted());
	EXPECT_FALSE(this->mockState1.deleted());
}

// -------------------------
template<class C>
class SubStateUnitTest : public UnitTestBase<C>
{
public:
	void SetUp() {
		UnitTestBase<C>::SetUp();
	}
	void TearDown() {
		EXPECT_HRESULT_SUCCEEDED(this->mockContext.shutdown());

		UnitTestBase<C>::TearDown();
	}
};

TYPED_TEST_SUITE(SubStateUnitTest, ContextTypes);

// State chain: State0 -> State1
// State1 returns State0 as next state.
TYPED_TEST(SubStateUnitTest, 0)
{
	{
		InSequence _sequence;
		EXPECT_CALL(this->mockState0, entry(&this->mockContext, nullptr, _)).WillOnce(Return(S_OK));
		EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
			.WillOnce(Return(S_OK));
		EXPECT_CALL(this->mockState0, handleEvent(&this->mockContext, &this->mockEvent, _))
			.WillOnce(DoAll(SetArgPointee<2>(&this->mockState1), Return(S_OK)));
		EXPECT_CALL(this->mockState1, entry(&this->mockContext, &this->mockEvent, &this->mockState0)).WillOnce(Return(S_OK));
		EXPECT_CALL(this->mockEvent, postHandle(&this->mockContext, S_OK))
			.WillOnce(Return(S_OK));
		EXPECT_CALL(this->mockEvent, preHandle(&this->mockContext))
			.WillOnce(Return(S_OK));
		EXPECT_CALL(this->mockState1, handleEvent(&this->mockContext, &this->mockEvent, _))
			.WillOnce(DoAll(SetArgPointee<2>(&this->mockState0), Return(S_OK)));
		EXPECT_CALL(this->mockState1, exit(&this->mockContext, &this->mockEvent, &this->mockState0)).WillOnce(Return(S_OK));
		EXPECT_CALL(this->mockEvent, postHandle(&this->mockContext, S_OK))
			.WillOnce(Return(S_OK));
	}
	EXPECT_CALL(this->mockState0, exit(_, _, _)).Times(0);

	ASSERT_EQ(S_OK, this->mockContext.setup(&this->mockState0));
	ASSERT_EQ(S_OK, this->mockContext.waitReady());

	this->mockState1.setMasterState(&this->mockState0);
	ASSERT_EQ(S_OK, this->mockContext.handleEvent(&this->mockEvent));	// State0 -> State1
	ASSERT_EQ(S_OK, this->mockContext.handleEvent(&this->mockEvent));	// State1 -> State0(Sub state goes back to master state)

	EXPECT_EQ(&this->mockState0, this->mockContext._getCurrentState());
	EXPECT_FALSE(this->mockState0.deleted());
	EXPECT_TRUE(this->mockState1.deleted());
	ASSERT_EQ(S_OK, this->mockContext.shutdown());
	EXPECT_TRUE(this->mockState0.deleted());
}

// Test for Context to ensure that async operations are denied.
class NotImplTest : public UnitTestBase<MockContext>
{
public:
	void SetUp() {
		UnitTestBase<MockContext>::SetUp();

		EXPECT_CALL(this->mockState0, entry(&this->mockContext, nullptr, _)).WillOnce(Return(S_OK));
		ASSERT_EQ(S_OK, this->mockContext.setup(&this->mockState0));
		ASSERT_EQ(S_OK, this->mockContext.waitReady());
	}
	void TearDown() {
		ASSERT_EQ(S_OK, this->mockContext.shutdown());

		UnitTestBase<MockContext>::TearDown();
	}
};

TEST_F(NotImplTest, 0)
{
	HRESULT hrExitCode;
	EXPECT_EQ(E_NOTIMPL, this->mockContext.getAsyncExitCode(&hrExitCode));

	MockEvent<MockContext> mockEvent;
	EXPECT_EQ(TSM_E_NOT_SUPPORTED, this->mockContext.triggerEvent(&mockEvent));

	EXPECT_TRUE(mockEvent.deleted());
}

class AssertUnitTest : public UnitTestBase<MockContext>
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

	auto hr = this->mockContext.handleEvent(&this->mockEvent);
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

	auto hr = this->mockContext.handleEvent(&this->mockEvent);
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
	void SetUp() {
		T::objectCount = 0;
	}
};

TYPED_TEST_SUITE(UnknownImplUnitTest, UnknownImplUnitTestTesteeTypes);

TYPED_TEST(UnknownImplUnitTest, create_delete)
{
	{
		CComPtr<TypeParam> testee;
		testee = new TypeParam();
		ASSERT_EQ(TypeParam::objectCount, 1);
		ASSERT_EQ(testee->impl.getCRef(), 1);
	}
	ASSERT_EQ(TypeParam::objectCount, 0);
}

TYPED_TEST(UnknownImplUnitTest, cRef)
{
	CComPtr<TypeParam> testee1(new TypeParam());
	CComPtr<TypeParam> testee2(testee1.p);

	ASSERT_EQ(testee1->impl.getCRef(), 2);
	ASSERT_EQ(TypeParam::objectCount, 1);
	testee1.Release();
	ASSERT_EQ(testee2->impl.getCRef(), 1);
	ASSERT_EQ(TypeParam::objectCount, 1);
	testee2.Release();
	ASSERT_EQ(TypeParam::objectCount, 0);
}


TYPED_TEST(UnknownImplUnitTest, QueryInterface)
{
	{
		CComPtr<TypeParam> testee(new TypeParam());
		CComPtr<IUnknown> unk;

		ASSERT_HRESULT_SUCCEEDED(testee.QueryInterface(&unk));
		ASSERT_EQ(testee, unk);
		ASSERT_EQ(testee->impl.getCRef(), 2);

		// Interface that TypeParam class does not implement.
		CComPtr<IRegistrar> reg;
		ASSERT_EQ(testee.QueryInterface(&reg), E_NOINTERFACE);

		ASSERT_EQ(testee.QueryInterface((IRegistrar**)nullptr), E_POINTER);
	}

	// All objects should be deleted.
	ASSERT_EQ(TypeParam::objectCount, 0);
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
