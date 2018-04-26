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
#include <log4cplus/fileappender.h>
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
//#include <log4cplus/helpers/loglog.h>
#include <exception>
#include <iostream>
#include <string>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

Logger * glog4logger = 0;
Logger glog4logger2 = Logger::getInstance(("testliblog4clplus"));

/* cant work!
 * undefined symbol: _ZN9log4cplus6LoggerC1EOS0_ */
//#define log4info(args...) LOG4CPLUS_INFO_FMT(Logger::getRoot(), ##args)
//#define log4info0(args...) LOG4CPLUS_INFO_FMT(Logger::getInstance(("testliblog4clplus")), ##args)

#define log4inf1(args...) LOG4CPLUS_INFO_FMT(*glog4logger, ##args)
#define log4info2(fmt, args...) LOG4CPLUS_INFO_FMT(*glog4logger, fmt, ##args)
#define log4info3(fmt, args...) LOG4CPLUS_INFO_FMT(glog4logger2, fmt, ##args)

int test_liblog4cplus_main(int argc, char ** argv)
{
    log4cplus::initialize ();
    std::auto_ptr<log4cplus::Layout> layout(new log4cplus::PatternLayout("[%D,%d{%q}][%p] [%l] %m%n"));
    SharedAppenderPtr append_1(new ConsoleAppender());
    append_1->setName(("testliblog4clplus"));
    append_1->setLayout(layout);
    Logger::getRoot().addAppender(append_1);

    SharedAppenderPtr append_2(
        new RollingFileAppender(LOG4CPLUS_TEXT("/tmp/testliblog4clplus.log"), 5*1024, 5));
    append_2->setName(LOG4CPLUS_TEXT("testliblog4clplus1"));
    append_2->setLayout( std::auto_ptr<Layout>(new TTCCLayout()) );
    Logger::getRoot().addAppender(append_2);

    Logger logger = Logger::getInstance(("testliblog4clplus"));

    LOG4CPLUS_INFO_FMT(logger,
                    LOG4CPLUS_TEXT (
                        "a log from log4cplus.%d, %d, %d, %ls...DONE"),
                    1, 2, 3, L"testing");

    LOG4CPLUS_INFO_FMT(logger, "another log from log4cplus, str='%s'", "hello");
    LogLog::getLogLog()->warn("a warn msg");

    Logger logger2 = Logger::getInstance(("testliblog4clplus"));
    glog4logger = &logger2;
    LOG4CPLUS_INFO_FMT(*glog4logger,
                        LOG4CPLUS_TEXT (
                            "LOG4CPLUS_INFO_FMT.%d, %d, %d, %ls...DONE"),
                        1, 2, 3, L"testing");
    LOG4CPLUS_TRACE_FMT(*glog4logger,
                            LOG4CPLUS_TEXT (
                                "LOG4CPLUS_TRACE_FMT.%d, %d, %d, %ls...DONE"),
                            1, 2, 3, L"testing");

    log4inf1("log4inf1: fuck log4cplus, str='%s'", "fuck");

    /* fuck cant work! */
//    log4info("fuck log4cplus");
//    log4info2("fuck log4cplus");
//    log4info0("fuck log4cplus, str='%s'", "fuck");

    log4info2("log4info2: fuck log4cplus, str='%s'", "fuck");
    log4info3("log4info3: fuck log4cplus, str='%s'", "fuck");


    log4cplus::threadCleanup ();
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
