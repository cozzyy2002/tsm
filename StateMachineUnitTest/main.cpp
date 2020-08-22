#include "stdafx.h"
#include "Mocks.h"
#include <log4cplus/initializer.h>
#include <log4cplus/configurator.h>

using namespace testing;

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("UnitTest.main"));

static TestEventListener* createTestEventListener(LPCSTR title);

int _tmain(int argc, TCHAR *argv[])
{
	// Initialize and ShutDown log4cplus.
	// This variable is never used.
	log4cplus::Initializer _log4cplus_Initializer;

	static auto logConfigFile = _T("log4cplus.properties");
	if(PathFileExists(logConfigFile)) {
		log4cplus::PropertyConfigurator::doConfigure(logConfigFile);
	} else {
		log4cplus::BasicConfigurator::doConfigure();
	}
	tsm::IContext::onAssertFailedProc = mockOnAssertFailed;

	InitGoogleMock(&argc, argv);

	// Register TestEventListener to log test case name, test name and test result.
	auto& listeners = UnitTest::GetInstance()->listeners();
	listeners.Append(createTestEventListener("StateMachineUnitTest"));

	return RUN_ALL_TESTS();
}

class TestEventLogger : public EmptyTestEventListener
{
public:
	TestEventLogger(LPCSTR title) : title(title) {}

	virtual void OnTestProgramStart(const UnitTest& unit_test) override {
		LOG4CPLUS_INFO(logger, "==== " << title.c_str() << " ====: Start");
	}
	virtual void OnTestCaseStart(const TestCase& test_case) override {
		LOG4CPLUS_INFO(logger, "---- Test case: " << test_case.name() << " start");
	}
	virtual void OnTestStart(const TestInfo& test_info) override {
		LOG4CPLUS_INFO(logger, "---- Test     : " << test_info.test_case_name() << "." << test_info.name() << " start");
	}
	virtual void OnTestPartResult(const TestPartResult& test_part_result) override {}
	virtual void OnTestEnd(const TestInfo& test_info) override {
		LOG4CPLUS_INFO(logger, "---- Test     : " << test_info.test_case_name() << "." << test_info.name()
								<< (test_info.result()->Passed() ? ": PASSED" : ": FAILED"));
	}
	virtual void OnTestCaseEnd(const TestCase& test_case) override {
		LOG4CPLUS_INFO(logger, "---- Test case: " << test_case.name() << " end");
	}
	virtual void OnTestProgramEnd(const UnitTest& unit_test) override {
		LOG4CPLUS_INFO(logger, "==== " << title.c_str() << " ====: Passed/Failed/Total: "
								<< unit_test.successful_test_count() << "/"
								<< unit_test.failed_test_count() << "/"
								<< unit_test.total_test_count());
	}

protected:
	std::string title;
};

TestEventListener* createTestEventListener(LPCSTR title)
{
	return new TestEventLogger(title);
}
