/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef PROCESS_UTIL_H_
#define PROCESS_UTIL_H_
#ifdef __cplusplus
extern "C"{
#endif

/*!
 * set termios display
 * @fd: STDIN_FILENO
 * @option 0 close
 */
int execute_with_wait_pipe(char * cmd, char * buff, int & length,
		void (*stdout_cb)(char const * data, int length, void * arg) = 0,
		void * arg = 0);

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /* PROCESS_UTIL_H_ */
