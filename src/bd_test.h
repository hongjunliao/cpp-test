/*
 * test_glib.h
 *
 *  Created on: Jan 5, 2016
 *      Author: root
 */

#ifndef TEST_H_
#define TEST_H_
#ifdef __cplusplus
extern "C"{
#endif
#define  TEST_CPP "cpp"
#define  TEST_BMP_IO "bmp_io"
#define  TEST_LIB_FREEIMAGE "libfreeimage"
#define  TEST_XLIB "xlib"
#define  SPICE_SERVER_CHANNEL_PLAYBACK "spice-server_channel_playback"
#define  TEST_LIB_PULSEAUDIO "libpulseaudio"
#define  TEST_LIB_H323PLUS	"libh323plus"
#define  TEST_LIB_SPICE_SERVER_DISPLAY_STREAMING	"libspice-server_display_streaming"

/*error code*/
#define BD_TEST_NO_SUCH_TEST -2

/*test_cpp.cpp, c++ language*/
int test_cpp_main(int argc, char **argv);
//test cario
int test_cairo_main(int argc, char **argv);
//test xlib
int test_xlib_main(int argc, char **argv);
//test libusb
int test_libusb_main(int argc, char **argv);
//test udev
int test_udev_main(int argc, char **argv);
//test xcb
int test_xcb_main(int, char **);
//!test epoll
int test_epoll_main(int, char **);
//curl
int test_curl_main(int, char **);
//json
int test_jsonc_main(int , char **	);
//usbredirfilter
int test_usbredirfilter_main(int, char **);
//gtk
int test_gtk_main(int, char **);
//
int lxrandr_0_3_1_main(int, char **);
//sice-server
int test_spice_server_main(int, char **);
//test_select.cpp, Linux select function
int test_select_main(int, char **);
//GIO g_io_add_watch
int test_gio_main(int, char **);
int test_coroutine_main(int, char **);
//bmp file io
int test_bmp_io_main(int, char **);
//libspice-client-gtk
int test_libspice_client_gtk_main(int, char **);
int test_gdkpixbuf_main(int, char **);
int test_ppm_to_bmp_main(int, char **);

//libfreeimage-dev
int test_libfreeimageplus_crop_main(int, char **);
//!
typedef int(*test_main_fn)(int argc, char * argv[]);

void test_main_add(test_main_fn);

/*test_libspice-server_channel_playback.c*/
int test_libspice_server_channel_playback_main(int argc, char ** argv);

/*test_pulseaudio.c*/
int test_pulseaudio_main(int argc, char ** argv);

/*test_h323.cpp*/
int test_h323_main(int argc, char ** argv) ;
/* from spice-0.13.1/server/test/test_display_streaming
 * test/spice_display_streaming/test_display_streaming.c*/
int test_spice_display_streaming_main(int argc, char **argv);
/**
 * 工具函数
 */
int xlib_draw_rectangle(int x, int y, int w, int h);

/*for test*/
int bd_test_main(int argc, char ** argv, char const * stest);
char const * bd_test_get_test_list();
#ifdef __cplusplus
}
#endif	//__cplusplus
#endif /* TEST_H_ */
