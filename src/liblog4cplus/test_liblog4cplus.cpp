/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/4/23
 *
 * test liblog4cplus
 *
 * (1)ubuntu: apt install liblog4cplus-dev
 * (2)gcc link: -llog4cplus

 * */
#ifndef WITHOUT_LIBLOG4CPLUS
#include <stdio.h>
#include <string.h> 	/* strlen */
#include <stdio.h>
#include <stdlib.h> 	/* calloc */
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */

#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/logger.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/thread/threads.h>
#include <log4cplus/helpers/sleep.h>
#include <log4cplus/streams.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/tracelogger.h>
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/loglevel.h"
#include <log4cplus/loggingmacros.h>
#include <exception>
#include <iostream>
#include <string>

using namespace std;
using namespace log4cplus;

int test_liblog4cplus_main(int argc, char ** argv)
{
    log4cplus::initialize ();
    std::auto_ptr<log4cplus::Layout> layout(new log4cplus::PatternLayout("[%D,%d{%q}][%p] [%l] %m%n"));
    SharedAppenderPtr append_1(new ConsoleAppender());
    append_1->setName(LOG4CPLUS_TEXT("testliblog4clplus"));
    append_1->setLayout(layout);
    Logger::getRoot().addAppender(append_1);
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("test"));

    LOG4CPLUS_INFO_FMT(logger,
                    LOG4CPLUS_TEXT (
                        "a log from log4cplus.%d, %d, %d, %ls...DONE"),
                    1, 2, 3, L"testing");
    return 0;
}

#else
int test_liblog4cplus_main(int argc, char ** argv)
{
	fprintf(stderr, "%s: undefine WITHOUT_LIBLOG4CPLUS and install liblog4cplus to enable this test\n"
			, __FUNCTION__);
	return -1;
}
#endif /* WITHOUT_LIBLOG4CPLUS */


char const * help_test_liblog4cplus()
{
	return "liblog4cplus";
}
