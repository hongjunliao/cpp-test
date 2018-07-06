/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2018/5/25
 *
 * eto: read or write until 'again' or error, using iovec
 * called epoll-style IO(ET mode)

 * like hp_pmrw, but more general
 * difference from hp_pmrw:
 * (1) use a new struct to manage the buffer's life cyle:
 * 		store the start address of buffer;
 *		record every write offset
 * (2) iovec NOT stored but generated every write
 * (3) I and O separated
 *
 * @see hp_pmrw
 * */

#ifndef XHHP_IO_H__
#define XHHP_IO_H__

#include <stddef.h>
#include <sys/uio.h>    /* iovec */
#include "list.h"       /* list_head */

#ifdef __cplusplus
extern "C" {
#endif

#define HP_ETIO_VEC 8

typedef	void  (* hp_eto_free_t)(void * ptr);

typedef struct hp_eto_item {
	void *             ptr;         /* ptr to free */
	hp_eto_free_t     free;        /* for free ptr */

	struct iovec       vec;         /* will change while writing */
	struct list_head   list;
} hp_eto_item;

typedef struct hp_eti {
	char *               i_buf;
	size_t               i_buflen;
	size_t               I_BUF_MAX;

	size_t               i_bytes;     /* total bytes read */
	/*
	 * callback for pack msg
	 * @return:
	 * EAGAIN: pack OK       --> continue reading for next pack
	 * else: pack failed     --> maybe error occurred, or NO more data needed, stop reading and return
	 * */
	int (* pack)(char * buf, size_t * len, void * arg);
	/*
	 * callback if tolled read done
	 *  */
	void (* read_done)(struct hp_eti * eti, int err, void * arg);
	/*
	 * callback for read error
	 *  */
	void (* read_error)(struct hp_eti * eti, int err, void * arg);
} hp_eti;

typedef struct hp_eto {
	struct hp_eto_item * o_items;
	int                  o_items_len;
	int                  O_ITEMS_LEN;
	size_t               o_bytes;     /* total bytes written */
	/*
	 * callback if write done
	 * */
	void (* write_done)(struct hp_eto * eto, int err, void * arg);

	/*
	 * callback if write error
	 * */
	void (* write_error)(struct hp_eto * eto, int err, void * arg);
} hp_eto;

int hp_eti_init(struct hp_eti * eti, int bufrlen);
void hp_eti_uninit(struct hp_eti * eti);

int hp_eto_init(struct hp_eto * eto, int n);
void hp_eto_uninit(struct hp_eto * eto);
int hp_eto_add(struct hp_eto * eto, void * iov_base, size_t iov_len, hp_eto_free_t free, void * ptr);
void hp_eto_reserve(struct hp_eto * eto, int n);
void hp_eto_clear(struct hp_eto * eto);

/* @see writev_a */
size_t hp_eto_write(struct hp_eto *eto, int fd, void * arg);
size_t hp_eti_read(struct hp_eti * eti, int fd, void * arg);

void hp_eio_uninit(struct hp_eto *eto);

#ifdef __cplusplus
}
#endif

#endif /* XHHP_IO_H__ */
