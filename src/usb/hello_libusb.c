/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>, @date 2017/12/19
 *
 * hello for libusb-dev
 */

#include "cpp_test.h"
#include <stdio.h>

#ifdef WITH_LIBUSB
#include "usb.h"
int hello_libusb_main(int argc, char * argv[])
{
	fprintf(stdout, "%s: TODO add impl\n", __FUNCTION__);
	return 0;
}
#else
int hello_libusb_main(int argc, char * argv[])
{
	fprintf(stderr, "%s: define WITH_LIBUSB to enable this test\n", __FUNCTION__);
	return -1;
}

#endif /* WITH_LIBUSB */

static char const * help_hello_libusb()
{
	return "hello_libusb";
}

struct test_entry hello_libusb = { "hello_libusb", hello_libusb_main, help_hello_libusb };
