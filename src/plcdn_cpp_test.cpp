#include <plcdn_cpp_test.h>
#include <algorithm>
#include <string>
#include <map>

//!
typedef int(*test_main_fn)(int argc, char * argv[]);

#define  TEST_CPP "cpp"
#define  TEST_BMP_IO "bmp_io"
#define  TEST_LIB_FREEIMAGE "libfreeimage"
#define  TEST_XLIB "xlib"
#define  SPICE_SERVER_CHANNEL_PLAYBACK "spice-server_channel_playback"
#define  TEST_LIB_PULSEAUDIO "libpulseaudio"
#define  TEST_LIB_H323PLUS	"libh323plus"
#define  TEST_LIB_SPICE_SERVER_DISPLAY_STREAMING	"libspice-server_display_streaming"

/* plcdn_la_ngx_log_fmt.cpp */
extern int test_ngx_parse_nginx_log_format_main(int argc, char ** argv);
/* test_binary_search_tree.cpp */
extern int test_binary_search_tree_main(int argc, char ** argv);
/* radix_tree.cpp */
extern int test_radix_tree_main(int argc, char ** argv);
/* bs23tree.cpp */
extern int test_bs23tree_main(int argc, char ** argv);
/* rb_tree.cpp */
extern int test_rbtree_main(int argc, char ** argv);
/* test_opengl.cpp */
extern int test_opengl_main(int argc, char ** argv);
/* graph.cpp */
extern int test_graph_main(int argc, char ** argv);
/* dynamic_programming.cpp */
extern int test_dynamic_programming_main(int argc, char ** argv);
/* test_string_util.cpp */
extern  int test_string_util_main(int argc, char ** argv);
/* ordered_2d_array_search.cpp */
extern int test_jzoffter_main(int argc, char ** argv);
/* test_string.cpp */
extern int test_string_main(int argc, char ** argv);
/* lds_main.cpp */
extern int test_lds_main(int argc, char ** argv);
/* exec.cpp */
extern int test_exec_main(int argc, char ** argv);

/* trie_st.cpp */
extern int test_trie_st_main(int argc, char ** argv);

/* chess_test.cpp */
extern int chess_test_main(int argc, char ** argv);

/* xhsdk_select_server.cpp */
extern int xhsdk_select_server_main(int argc, char ** argv);

/* tcp_echo_cli.cpp */
extern int tcp_echo_cli_main(int argc, char ** argv);

/* mem_pool.cpp */
extern int mp_test_main(int argc, char ** argv);

/* test_zlib.cpp */
extern int test_zlib_main(int argc, char ** argv);

static std::map<std::string, test_main_fn> testmap = {
#ifdef __CYGWIN_GCC__

			{"xcb",test_xcb_main},
			{"cairo",test_cairo_main},
			{"xlib",test_xlib_main},
			{"libusb",test_libusb_main},
			{"udev",test_udev_main},
			{TEST_CPP,test_cpp_main},
			{TEST_BMP_IO,test_bmp_io_main},
			{"fifo",test_cpp_main},
			{"curl",test_curl_main},
			{"json",test_jsonc_main},
			{"usbredirfilter",test_usbredirfilter_main},
			{"gtk",test_gtk_main},
			{"select",test_select_main},
			{"spice-server",test_spice_server_main},
			{SPICE_SERVER_CHANNEL_PLAYBACK,test_libspice_server_channel_playback_main},
			{"gio",test_gio{"radix", test_radix_tree_main},_main},
			{"coroutine",test_coroutine_main},
			{"libspice-client-gtk",test_libspice_client_gtk_main},
			{"gdkpixbuf",test_gdkpixbuf_main},
			{"ppm-to-bmp",test_ppm_to_bmp_main},
			{TEST_LIB_FREEIMAGE, test_libfreeimageplus_crop_main},
			{TEST_LIB_PULSEAUDIO, test_pulseaudio_main},
			{TEST_LIB_H323PLUS, test_h323_main},
			{TEST_LIB_SPICE_SERVER_DISPLAY_STREAMING, test_spice_display_streaming_main},
			{"win32_pipe", test_win32_pipe_main},
			{"subprocess_with_pipe", test_subprocess_with_pipe_main},
			{"subprocess_wait_pipe_handle", test_subprocess_wait_pipe_handle_main},
			{"mykeys", mykeys_main},
			{"cygwin_posix_ipc", test_cygwin_posix_ipc_main},
			{"nginx_log_split", test_nginx_log_split_main},
			{"srs_log_stats", test_srs_log_stats_main},
			{"plcdn_log_analysis", test_plcdn_log_analysis_main},
			{"plcdn_log_split", test_plcdn_log_split_main},
			{"opencv", test_opencv_main},
			{"opengl", test_opengl_main},
			{"nginx_log_fmt", test_ngx_parse_nginx_log_format_main},
			{"plcdn_logtrans", test_nginx_transform_log_main_2},
			{"chess", chess_test_main},
#endif /*__CYGWIN_GCC__*/
			{"cpp11", test_cpp11_main},
			{"bst", test_binary_search_tree_main},
			{"radix", test_radix_tree_main},
			{"bs23t", test_bs23tree_main},
			{"rbtree", test_rbtree_main},
			{"graph", test_graph_main},
			{"dyn_p", test_dynamic_programming_main},
			{"string_util", test_string_util_main},
			{"jzoffer", test_jzoffter_main},
			{"string", test_string_main},
			{"trie_st", test_trie_st_main},
			{"xhsdksrv", xhsdk_select_server_main},
			{"tcpechocli", tcp_echo_cli_main},
			{"mempool", mp_test_main},
			{"zlib", test_zlib_main},
			/*{"lds", test_lds_main},*/
			/*{"lxrandr",lxrandr_0_3_1_main},*/
#ifdef WIN32
			{"exec", test_exec_main },
#endif /* WIN32 */
};

char const * bd_test_get_test_list()
{
	char const * ret = "";
	static std::string s = "[help|--help|-h";
	for(auto iter = testmap.begin(); iter != testmap.end(); ++iter){
		s += "|";
		s += iter->first;
	}
	s += "]";
	ret = s.c_str();
	return ret;
}

int bd_test_main(int argc, char ** argv, char const * stest)
{
	setbuf(stdout, 0);
	if(!stest)
		return -1;
	auto fn = testmap[stest];
	if(!fn){
		fprintf(stderr, "%s: no such test: %s\n", __FUNCTION__, stest);
		return BD_TEST_NO_SUCH_TEST;
	}
	return fn(argc, argv);
}
