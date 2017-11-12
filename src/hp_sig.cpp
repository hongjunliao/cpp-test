/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2017/9/13
 *
 * signal and handler
 * NOTE: the main code is from redis/server.c, redis: https://github.com/antirez/redis
 * copyright below:
 *
 * Copyright (c) 2009-2016, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 */
#include <unistd.h>   /* getpid */
#include <signal.h>  /* sigaction, ... */
#include <stdlib.h>  /* exit */
#include <stdio.h>   /* */
#include <string.h>  /* */
#include <time.h>    /* */
/* Log a fixed message without printf-alike capabilities, in a way that is
 * safe to call from a signal handler.
 *
 * We actually use this only for signals that are not fatal from the point
 * of view of Redis. Signals that are going to kill the server anyway and
 * where we need printf-alike features are served by serverLog(). */
static void serverLogFromHandler(const char *msg) {
    int fd = fileno(stdout);
    char buf[64];

    if (fd == -1) return;
    sprintf(buf,"%d",getpid());
    if (write(fd,buf,strlen(buf)) == -1) goto err;
    if (write(fd,":signal-handler (",17) == -1) goto err;
    sprintf(buf, "%ld",time(NULL));
    if (write(fd,buf,strlen(buf)) == -1) goto err;
    if (write(fd,") ",2) == -1) goto err;
    if (write(fd,msg,strlen(msg)) == -1) goto err;
    if (write(fd,"\n",1) == -1) goto err;
err:
	return;
}

static void sigShutdownHandler(int sig) {
    char const * msg;
    int is_chld = 0;

    switch (sig) {
    case SIGINT:
        msg = "Received SIGINT scheduling shutdown...";
        break;
    case SIGTERM:
        msg = "Received SIGTERM scheduling shutdown...";
        break;
    case SIGCHLD:
    	msg = "Received SIGCHLD, call wait";
    	is_chld = 1;
    	wait();
    	break;
    default:
        msg = "Received shutdown signal, scheduling shutdown...";
    };

    /* SIGINT is often delivered via Ctrl+C in an interactive session.
     * If we receive the signal the second time, we interpret this as
     * the user really wanting to quit ASAP without waiting to persist
     * on disk. */
    serverLogFromHandler(msg);
    if(!is_chld)
    	exit(0);
}

void setupSignalHandlers(void) {
    struct sigaction act;

    /* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction is used.
     * Otherwise, sa_handler is used. */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sigShutdownHandler;
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);

#ifdef HAVE_BACKTRACE
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
    act.sa_sigaction = sigsegvHandler;
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGILL, &act, NULL);
#endif
    return;
}
