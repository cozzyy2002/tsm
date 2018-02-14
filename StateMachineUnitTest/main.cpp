#include "stdafx.h"
#include "Mocks.h"
#include <log4cplus/configurator.h>

using namespace testing;

int _tmain(int argc, TCHAR *argv[])
{
	log4cplus::BasicConfigurator::doConfigure();
	tsm::IContext::onAssertFailedProc = mockOnAssertFailed;

	InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
