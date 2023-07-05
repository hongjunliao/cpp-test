/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/7/23
 *
 * test for libspdlog-dev
 * */
#ifdef CPP_TEST_WITH_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
int test_spdlog_main(int argc, char ** argv)
{
	auto console = spdlog::stdout_color_mt("console");
	return 0;
}
#endif
