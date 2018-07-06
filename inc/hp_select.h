/*!
 * This file is PART of xhhp project
 * @author hongjun.liao <docici@126.com>, @date 2018/5/18
 *
 * the select event system
 * */

#ifndef XHHP_SELECT_H__
#define XHHP_SELECT_H__

#include <sys/select.h> /* fd_set */

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////

/* fd_sets and following macros used for e.g. select */
struct fd_sets {
	int maxfd;
	fd_set rfds, wfds;
};

#define FDS_ZERO(fds)                                            \
    do{ (fds)->maxfd = -1;                                       \
         FD_ZERO(&(fds)->rfds); FD_ZERO(&(fds)->wfds);           \
    }while(0)

#define FDS_SET_RFD(fd, fds)                                     \
	do{ (fds)->maxfd = ((fds)->maxfd > (fd)? (fds)->maxfd :(fd));\
        FD_SET((fd), &((fds)->rfds));                            \
	}while(0)

#define FDS_SET_WFD(fd, fds)                                     \
	do{ (fds)->maxfd = ((fds)->maxfd > (fd)? (fds)->maxfd :(fd));\
        FD_SET((fd), &((fds)->wfds));                            \
	}while(0)

#define FDS_ISSET_RFD(fd, fds)     (FD_ISSET((fd), &((fds)->rfds)))
#define FDS_ISSET_WFD(fd, fds)     (FD_ISSET((fd), &((fds)->wfds)))

/////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* XHHP_SELECT_H__ */
