/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef PROCESS_UTIL_H_
#define PROCESS_UTIL_H_

#ifndef WIN32
#define WIN32
#endif

#if (defined __CYGWIN__ || defined WIN32)
#include <windows.h>
#endif /* (defined __CYGWIN__ || defined WIN32) */

#ifdef __cplusplus
extern "C"{
#endif

/*!
 * set termios display
 * @fd: STDIN_FILENO
 * @option 0 close
 */
int execute_with_wait_pipe(char const * dir, char * cmd, char * buff, int & length,
		void (*stdout_cb)(char const * data, int length, void * arg) = 0,
		void * arg = 0);

/*
 * 交互式命令/interactive_cmd, intercmd, 交互式命令是指需要提输入的命令, 如sqlcmd, shell, gdb
 * (1)程序启动后, 不会马上退出, 而是等待输入指令(directive), 并按下回车键(通常)以执行该指令;
 * (2)如果输入的指令是"退出"(如'quit', 'exit', 'bye'), 则程序退出
 * (3)如果是其它指令, 执行并输出结果, 如sqlcmd的查询语句
 * */

struct intercmdctx {
	char const * dir;	 /* woking dir for cmd */
	char const * cmd;    /* the cmd */
	char const * direct; /* direct to send */
	bool exit;           /* if direct is an exit direct then true */
/* reset these members to 0 before use intercmdctx */
#if (defined __CYGWIN__ || defined WIN32)
	HANDLE hproc;        /* process handle of the cmd */
	HANDLE rh;           /* read pipe handle */
	HANDLE wh;           /* write pipe handle */
#endif /* (defined __CYGWIN__ || defined WIN32) */
};

/* send directive
 * send 'exit' like directive to exit cmd
 * @NOTE:
 * (1)当前该函数存在BUG,不能正确返回指令的输出, 返回值仅表示指令已被正确发送到@param ctx
 * */
int send_intercmd(intercmdctx & ctx, char * buf, size_t length,
		int (*stdout_cb)(char const * data, size_t length, void * arg), void * arg);

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /* PROCESS_UTIL_H_ */
