/**
 * from libuv/docs/uvwget/main.c
*/
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "hp/sdsinc.h"
#if defined(CPP_TEST_WITH_UV) && defined(CPP_TEST_WITH_CURL)

#include <uv.h>
#include <curl/curl.h>

static uv_loop_t *loop;
static CURLM *curl_handle;
static uv_timer_t timeout;

#define TEST_URL "https://download.electrum.org/4.4.5/electrum-4.4.5-x86_64.AppImage"
#define TEST_SHA256 "ad341c9c47c7685acbbcf9c8d48665f6e9360f43452e7d0f49a1ac23d2b22ebe"
#define TEST_FSIZE 62534848
#define TEST_FILE __FUNCTION__

typedef struct hp_uv_curlm hp_uv_curlm;

/* callback
 *	 bytes:   bytes received
 *   content_length:   total bytes of body if has "Content-Length"
 *   resp:    @see hp_uv_curlm_add
 *   arg:     @see hp_uv_curlm_add
 */
typedef int (* hp_uv_curl_proress_cb_t)(int bytes, int content_length, sds resp, void * arg);
typedef int (* hp_uv_curl_done_cb_t)(hp_uv_curlm * curlm, char const * url, sds str, void * arg);


struct hp_uv_curlm {
	CURLM *     curl_handle;        /* the libcurl multi handle */
	uv_timer_t  timeout;
	uv_loop_t * loop;
};

typedef struct hp_uv_curlm_easy {
	CURL * 		curl;
	uv_poll_t 	poll_handle;
	curl_socket_t fd;

	sds      	resp;   /* the response */
	FILE *      f;      /* the response, write to file? */

	/* for progress */
	int 	    bytes;
	int 	    content_length;

	hp_uv_curl_proress_cb_t on_proress;
	hp_uv_curl_done_cb_t on_done;
	void *      arg;

	hp_uv_curlm * curlm;  /* ref to context */

	struct curl_slist * hdrs;
#if (LIBCURL_VERSION_MAJOR >=7 && LIBCURL_VERSION_MINOR >= 56)
	curl_mime *	form;
#else
	struct curl_httppost * form;
#endif /* LIBCURL_VERSION_MINOR */
} hp_uv_curlm_easy;

static hp_uv_curlm_easy *create_curl_context(curl_socket_t sockfd) {
    hp_uv_curlm_easy *context;

    context = (hp_uv_curlm_easy*) malloc(sizeof *context);

    context->fd = sockfd;

    int r = uv_poll_init_socket(loop, &context->poll_handle, sockfd);
    assert(r == 0);
    context->poll_handle.data = context;

    return context;
}

static void curl_close_cb(uv_handle_t *handle) {
    hp_uv_curlm_easy *context = (hp_uv_curlm_easy*) handle->data;
    free(context);
}

static void destroy_curl_context(hp_uv_curlm_easy *context) {
    uv_close((uv_handle_t*) &context->poll_handle, curl_close_cb);
}


static void add_download(const char *url, int num) {
    char filename[50];
    sprintf(filename, "%d.download", num);
    FILE *file;

    file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening %s\n", filename);
        return;
    }

    CURL *handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_multi_add_handle(curl_handle, handle);
    fprintf(stderr, "Added download %s -> %s\n", url, filename);
}

static void check_multi_info(void) {
    char *done_url;
    CURLMsg *message;
    int pending;

    while ((message = curl_multi_info_read(curl_handle, &pending))) {
        switch (message->msg) {
        case CURLMSG_DONE:
            curl_easy_getinfo(message->easy_handle, CURLINFO_EFFECTIVE_URL,
                            &done_url);
            printf("%s DONE\n", done_url);

            curl_multi_remove_handle(curl_handle, message->easy_handle);
            curl_easy_cleanup(message->easy_handle);
            break;

        default:
            fprintf(stderr, "CURLMSG default\n");
            abort();
        }
    }
}

static void curl_perform(uv_poll_t *req, int status, int events) {
    uv_timer_stop(&timeout);
    int running_handles;
    int flags = 0;
    if (status < 0)                      flags = CURL_CSELECT_ERR;
    if (!status && events & UV_READABLE) flags |= CURL_CSELECT_IN;
    if (!status && events & UV_WRITABLE) flags |= CURL_CSELECT_OUT;

    hp_uv_curlm_easy *context;

    context = (hp_uv_curlm_easy*)req;

    curl_multi_socket_action(curl_handle, context->fd, flags, &running_handles);
    check_multi_info();   
}

static void on_timeout(uv_timer_t *req) {
    int running_handles;
    curl_multi_socket_action(curl_handle, CURL_SOCKET_TIMEOUT, 0, &running_handles);
    check_multi_info();
}

static void start_timeout(CURLM *multi, long timeout_ms, void *userp) {
    if (timeout_ms <= 0)
        timeout_ms = 1; /* 0 means directly call socket_action, but we'll do it in a bit */
    uv_timer_start(&timeout, on_timeout, timeout_ms, 0);
}

static int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) {
    hp_uv_curlm_easy *curl_context;
    if (action == CURL_POLL_IN || action == CURL_POLL_OUT) {
        if (socketp) {
            curl_context = (hp_uv_curlm_easy*) socketp;
        }
        else {
            curl_context = create_curl_context(s);
            curl_multi_assign(curl_handle, s, (void *) curl_context);
        }
    }

    switch (action) {
        case CURL_POLL_IN:
            uv_poll_start(&curl_context->poll_handle, UV_READABLE, curl_perform);
            break;
        case CURL_POLL_OUT:
            uv_poll_start(&curl_context->poll_handle, UV_WRITABLE, curl_perform);
            break;
        case CURL_POLL_REMOVE:
            if (socketp) {
                uv_poll_stop(&((hp_uv_curlm_easy*)socketp)->poll_handle);
                destroy_curl_context((hp_uv_curlm_easy*) socketp);
                curl_multi_assign(curl_handle, s, NULL);
            }
            break;
        default:
            abort();
    }

    return 0;
}

int test_uv_main(int argc, char **argv) {
    loop = uv_default_loop();

    argc = 2; argv[1] = TEST_URL;

    if (argc <= 1)
        return 0;

    if (curl_global_init(CURL_GLOBAL_ALL)) {
        fprintf(stderr, "Could not init cURL\n");
        return 1;
    }

    uv_timer_init(loop, &timeout);

    curl_handle = curl_multi_init();
    curl_multi_setopt(curl_handle, CURLMOPT_SOCKETFUNCTION, handle_socket);
    curl_multi_setopt(curl_handle, CURLMOPT_TIMERFUNCTION, start_timeout);

    while (argc-- > 1) {
        add_download(argv[argc], argc);
    }

    uv_run(loop, UV_RUN_DEFAULT);
    curl_multi_cleanup(curl_handle);
    return 0;
}
#endif //#ifdef CPP_TEST_WITH_LIBUV
