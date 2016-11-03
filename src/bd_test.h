/*!
 * include file for cpp-test
 * @author hongjun.liao<docici@126.com>
 */

#ifndef CPP_TEST_H_
#define CPP_TEST_H_
#ifdef __cplusplus
extern "C"{
#endif
/*test_cpp.cpp, c++ language*/
int test_cpp_main(int argc, char **argv);
/*test_cpp11.cpp, c++11*/
int test_cpp11_main(int argc, char **argv);
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

/*test_libspice-server_channel_playback.c*/
int test_libspice_server_channel_playback_main(int argc, char ** argv);

/*test_pulseaudio.c*/
int test_pulseaudio_main(int argc, char ** argv);

/*test_h323.cpp*/
int test_h323_main(int argc, char ** argv) ;
/* from spice-0.13.1/server/test/test_display_streaming
 * test/spice_display_streaming/test_display_streaming.c*/
int test_spice_display_streaming_main(int argc, char **argv);

int test_cygwin_posix_ipc_main(int argc, char **argv);
/*test_win32_pipe.cpp*/
int test_win32_pipe_main(int argc, char ** argv);
int test_subprocess_with_pipe_main(int argc, char ** argv);
int test_subprocess_wait_pipe_handle_main(int argc, char ** argv);

/*mykeys_main.cpp*/
int mykeys_main(int argc, char ** argv);

/*nginx_log_analysis/main.cpp*/
int test_nginx_log_split_main(int argc, char ** argv);
int test_nginx_log_stats_main(int argc, char ** argv);
/*srs_log_analysis/main.cpp*/
int test_srs_log_stats_main(int argc, char ** argv);
/**
 * xlib util
 */
int xlib_draw_rectangle(int x, int y, int w, int h);

/*mykeys.cpp*/


/*!
 * @param sep, seperator: "\r\n"
 */
int mykeys_regex_search(char const * key_file, char const * password, char const * keyword,
		char const * sep, char * matches, int len);
/*error code*/
#define BD_TEST_NO_SUCH_TEST -2

/*call test, no @param stest then return @see BD_TEST_NO_SUCH_TEST*/
int bd_test_main(int argc, char ** argv, char const * stest);
char const * bd_test_get_test_list();
#ifdef __cplusplus
}
#endif	//__cplusplus
#endif /* CPP_TEST_H_ */
