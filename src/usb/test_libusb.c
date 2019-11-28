/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>, @date 2017/12/19
 *
 * install:
 * libusb-1.0-0-dev
 * */

/*
 * libusb example program to list devices on the bus
 * Copyright © 2007 Daniel Drake <dsd@gentoo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <stdio.h>

#ifdef WITH_LIBUSB

#include "libusb.h"

int test_write_txt_main(int argc, char * argv[])
{

	libusb_context * ctx;
	libusb_init(&ctx);
	libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_DEBUG);
	uint16_t usbvendor = 0x0781, usbproduct = 0x5567;

	libusb_device_handle * handle = libusb_open_device_with_vid_pid(ctx, usbvendor, usbproduct);
	if (!handle) {
		printf("libusb_open_device_with_vid_pid failed!\n");
		libusb_exit(ctx);
		return 1;
	}
//	int LIBUSB_CALL libusb_bulk_transfer(libusb_device_handle *dev_handle,
//		unsigned char endpoint, unsigned char *data, int length,
//		int *actual_length, unsigned int timeout);
	unsigned char endpoint  = 0;
	unsigned char *data = NULL;
	int length = 0;
	int actual_length = 0;
	unsigned int timeout = 0;

	int r = libusb_bulk_transfer(handle, endpoint, data, length, &actual_length, timeout);
	if(0 != r){
		printf("libusb_bulk_transfer failed! error = %d\n", r);
		libusb_exit(ctx);
		return 1;
	}
	libusb_exit(ctx);

	return 0;
}
int test_libusb_main(int argc, char * argv[])
{
	libusb_device **devs;
	int r;
	ssize_t cnt;

	r = libusb_init(NULL);
	if (r < 0)
		return r;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return (int) cnt;

	libusb_device *dev;
	int i = 0, j = 0;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return 1;
		}

		printf("%04x:%04x (bus %d, device %d)",
			desc.idVendor, desc.idProduct,
			libusb_get_bus_number(dev), libusb_get_device_address(dev));

//		r = libusb_get_port_numbers(dev, path, sizeof(path));
//		if (r > 0) {
//			printf(" path: %d", path[0]);
//			for (j = 1; j < r; j++)
//				printf(".%d", path[j]);
//		}
		printf("\n");
	}
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);

	return 0;
}
#else
int test_libusb_main(int argc, char * argv[])
{
	fprintf(stderr, "%s: define WITH_LIBUSB to enable this test\n", __FUNCTION__);
	return -1;
}
#endif /* WITH_LIBUSB */

static char const * help_libusb()
{
	return "libusb_1_0";
}

