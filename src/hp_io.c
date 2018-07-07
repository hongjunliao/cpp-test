/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2018/5/16
 *
 * the proxy I/O
 * */
#include "hp_io.h"
#include <unistd.h>     /* read, sysconf, ... */
#include <stdio.h>
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */
#include <stdlib.h>
#include <limits.h>		/* IOV_MAX */

#define hp_min(a, b)                              ((a) < (b)? (a):(b))

extern int gloglevel;
/////////////////////////////////////////////////////////////////////////////////////
int hp_eto_init(struct hp_eto * eto, int n)
{
	if(!eto)
		return -1;
	memset(eto, 0, sizeof(struct hp_eto));

	eto->O_ITEMS_LEN = n;
	eto->o_items = (struct hp_eto_item *)malloc(eto->O_ITEMS_LEN * sizeof(struct hp_eto_item));

	return 0;
}

void hp_eto_uninit(struct hp_eto * eto)
{
	if(!eto)
		return;
	hp_eto_clear(eto);
	free(eto->o_items);
}

int hp_eti_init(struct hp_eti * eti, int bufrlen)
{
	if(!eti)
		return -1;
	memset(eti, 0, sizeof(struct hp_eti));

	if(bufrlen <= 0)
		bufrlen = 1024 * 8;
	eti->I_BUF_MAX = bufrlen;

	return 0;
}

void hp_eti_uninit(struct hp_eti * eti)
{
	if(!eti)
		return;

	free(eti->i_buf);
}

int hp_eto_add(struct hp_eto * eto, void * iov_base, size_t iov_len, hp_eto_free_t free, void * ptr)
{
	if(!(eto && iov_base))
		return -1;

	if(eto->O_ITEMS_LEN - eto->o_items_len <= 2){
		if(gloglevel > 8)
			hp_log(stdout, "%s: hp_eto::o_items_len reached max, MAX=%d, realloc\n"
				, __FUNCTION__, eto->O_ITEMS_LEN);

		/* @see https://www.zhihu.com/question/36538542 */
		hp_eto_reserve(eto, eto->O_ITEMS_LEN * 2);
	}

	struct hp_eto_item * item = eto->o_items + eto->o_items_len;

	item->ptr = ptr;
	if(!item->ptr)
		item->ptr = iov_base;
	item->free = free;

	item->vec.iov_base = iov_base;
	item->vec.iov_len = iov_len;

	++eto->o_items_len;

	return 0;
}

void hp_eto_reserve(struct hp_eto * eto, int n)
{
	if(eto->O_ITEMS_LEN - eto->o_items_len >= n)
		return;

	eto->O_ITEMS_LEN += n;
	eto->o_items = (struct hp_eto_item *)realloc(eto->o_items, eto->O_ITEMS_LEN * sizeof(struct hp_eto_item));
}

static void hp_etio_del_n(struct hp_eto *etio, int n)
{
	int i;
	for(i = 0; i < n; ++i){
		struct hp_eto_item * item = etio->o_items + i;
		if(item->free && item->ptr){
			item->free(item->ptr);
		}
	}
	memmove(etio->o_items, etio->o_items + n, sizeof(struct hp_eto_item) * (etio->o_items_len - n));
	etio->o_items_len -= n;
}

void hp_eto_clear(struct hp_eto * eto)
{
	return hp_etio_del_n(eto, eto->o_items_len);
}

size_t hp_eto_write(struct hp_eto *etio, int fd, void * arg)
{
	if(!etio)
		return 0;

	if(etio->o_items_len == 0)
		return 0;

	int iov_max = sysconf(_SC_IOV_MAX);
	if(iov_max <= 0)
		iov_max = IOV_MAX;

	size_t o_bytes = 0; /* out bytes this time */
	for(;;){
		/* all write done, return */
		if(etio->o_items_len == 0){
			if(etio->write_done)
				etio->write_done(etio, 0, arg);
			return o_bytes;
		}

		int vec_len = hp_min(iov_max, etio->o_items_len);
		struct iovec * vec = calloc(vec_len, sizeof(struct iovec));

		/* prepare data for iovec */
		int i;
		for(i = 0; i < vec_len; ++i){
			struct hp_eto_item * item = etio->o_items + i;
			vec[i] = item->vec;
		}
		size_t W = iovec_total_bytes(vec, vec_len);  /* bytes to write this loop */
		if(W == 0){
			free(vec);
			hp_etio_del_n(etio, vec_len);
			continue;
		}

		/* now time to write */
		int n = 0, err = 0;
		ssize_t w = writev_a(fd, &err, vec, vec_len, &n, W);
		etio->o_bytes += w;
		o_bytes += w;

		/* adjust the last one for next write */
		if(n < vec_len)
			etio->o_items[n].vec = vec[n];

		/* free iovec */
		free(vec);

		/* clear these written ones: [0,n) */
		hp_etio_del_n(etio, n);

		/* now check the write result  */
		if(err == 0){
			/* all written this time!  go to next write loop */
			continue;
		}
		else if(err == EAGAIN){
			/* resources NOT available this moment,
			 * call this function later */
			break;
		}
		else {
#ifndef NDEBUG
			if(gloglevel > 0)
				hp_log(stderr, "%s: writev_a ERROR, fd=%d, return=%d, errno=%d/'%s'\n", __FUNCTION__
					, fd, err, errno, strerror(errno));
#endif /* NDEBUG */
			/* write error occurred, the fd expect to be reset */
			if(etio->write_error)
				etio->write_error(etio, err, arg);
			break;
		}
	} /* end of write loop */

	return o_bytes;
}

size_t hp_eti_read(struct hp_eti * eti, int fd, void * arg)
{
	if(!(eti && eti->pack))
		return 0;

	if(!eti->i_buf){
		eti->i_buf = (char * )malloc(eti->I_BUF_MAX);
		assert(eti->i_buf);
	}

	size_t i_bytes = 0; /* in bytes this time */
	for(;;){
		int r = eti->pack(eti->i_buf, &eti->i_buflen, arg);
		if(r != EAGAIN){
			if(eti->read_done)
				eti->read_done(eti, r, arg);
			break;
		}

		if(!(eti->i_buflen < eti->I_BUF_MAX)){
			hp_log(stderr, "%s: i_buf full, too small? i_buf/MAX=%zu/%zu\n", __FUNCTION__
					, eti->i_buflen, eti->I_BUF_MAX);
			break;
		}

		ssize_t n = read(fd, eti->i_buf + eti->i_buflen, eti->I_BUF_MAX - eti->i_buflen);
		if(n > 0){
			eti->i_bytes += n;
			eti->i_buflen += n;
			i_bytes += n;

			continue; /* read some, try to pack */
		}
		else if(n < 0) {
			if((errno == EINTR || errno == EAGAIN)){
				/* read again later */
			}
			else{
#ifndef NDEBUG
			if(gloglevel > 0)
				hp_log(stderr, "%s: read ERROR, fd=%d, return=%d, errno=%d/'%s'\n", __FUNCTION__
					, fd, n, errno, strerror(errno));
#endif /* NDEBUG */
				if(eti->read_error)
					eti->read_error(eti, errno, arg);
			}
			break;
		}
		else if(n == 0){ /* EOF */
#ifndef NDEBUG
			if(gloglevel > 9)
				hp_log(stdout, "%s: read EOF, fd=%d, return=%d, errno=%d/'%s'\n", __FUNCTION__
					, fd, n, errno, strerror(errno));
#endif /* NDEBUG */
			if(eti->read_error)
				eti->read_error(eti, 0, arg);
			break;
		}
	} /* loop for read and parse */

	return i_bytes;
}

/////////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG

int test_hp_io_main(int argc, char ** argv)
{
	struct hp_eto pioobj, * etio = &pioobj;
	int r = hp_eto_init(etio, HP_ETIO_VEC);
	assert(r == 0);

	hp_eto_write(etio, 1, 0);
	hp_eto_uninit(etio);
	return -1;
}
#endif /* NDEBUG */


