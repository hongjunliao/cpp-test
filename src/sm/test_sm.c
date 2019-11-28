/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/11/14
 *
 * test sm/state machine
 * code form:
 * https://stackoverflow.com/questions/1371460/state-machines-tutorials
 * */
#include <stdio.h>
#include <string.h> 	/* strlen */
#include <stdlib.h> 	/* calloc */
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */

int entry_state(void){ return 0; }
int foo_state(void){ return 0; }
int bar_state(void){ return 0; }
int exit_state(void){ return 0; }

/* array and enum below must be in sync! */
int (* state[])(void) = { entry_state, foo_state, bar_state, exit_state};
enum state_codes { entry, foo, bar, end};

enum ret_codes { ok, fail, repeat};
struct transition {
    enum state_codes src_state;
    enum ret_codes   ret_code;
    enum state_codes dst_state;
};
/* transitions from end state aren't needed */
struct transition state_transitions[] = {
    {entry, ok,     foo},
    {entry, fail,   end},
    {foo,   ok,     bar},
    {foo,   fail,   end},
    {foo,   repeat, foo},
    {bar,   ok,     end},
    {bar,   fail,   end},
    {bar,   repeat, foo}};

#define EXIT_STATE end
#define ENTRY_STATE entry

enum state_codes lookup_transitions(enum state_codes cur_state, enum ret_codes rc)
{
	return cur_state;
}

int test_sm_main(int argc, char *argv[])
{
    enum state_codes cur_state = ENTRY_STATE;
    enum ret_codes rc;
    int (* state_fun)(void);

    for (;;) {
        state_fun = state[cur_state];
        rc = state_fun();
        if (EXIT_STATE == cur_state)
            break;
        cur_state = lookup_transitions(cur_state, rc);
    }

    return 0;
}

char const * help_test_sm()
{
	return "libprotobuf_libev";
}
