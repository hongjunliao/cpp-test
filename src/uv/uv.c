/*
 * uv.cpp
 *
 *  Created on: Jun 28, 2023
 *      Author: jun
 */
#include "config.h"
#ifdef WITH_LIBUV
#include "cpp_test.h"
//#if 1
#include <stdio.h>
#include <uv.h>
#include "hp/hp_log.h"

static void on_read(uv_fs_t *req) {
    if (req->result >= 0) {
        printf("Read: %s\n", (char *)req->data);
    } else {
        fprintf(stderr, "Read error: %s\n", uv_strerror((int)req->result));
    }

    uv_fs_req_cleanup(req);
    free(req);
}

static void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_open(uv_fs_t *req) {
    if (req->result >= 0) {
        uv_fs_t *read_req = (uv_fs_t*)malloc(sizeof(uv_fs_t));
        uv_fs_read(uv_default_loop(), read_req, req->result, read_req, 1, -1, on_read);
    } else {
        fprintf(stderr, "Open error: %s\n", uv_strerror((int)req->result));
    }

    uv_fs_req_cleanup(req);
    free(req);
}

int test_uv_main(int argc, char ** argv)
{
	hp_log(stdout, "%s:openning file %s\n", __FUNCTION__, argc > 1? argv[1] : "(null)");
	if(argc <= 1) return -1;

    uv_loop_t *loop = uv_default_loop();

    uv_fs_t *open_req = (uv_fs_t*)malloc(sizeof(uv_fs_t));
    uv_fs_open(loop, open_req, "file.txt", O_RDONLY, 0, on_open);

    return uv_run(loop, UV_RUN_DEFAULT);
}


#endif // WITH_LIBUV
