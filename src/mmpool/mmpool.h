/*!
 * a simple memory pool, for allocation of  lots of small objects
 * @author hongjun.liao <docici@126.com>, @date 2017/12/25
 */

struct mmpool;

/* @param objsize > 0, or return NULL, @param factor [0-10] */
struct mmpool * mmp_create(int objsize, int factor);

void * mmp_alloc(struct mmpool * mp);
void mmp_free(void * p);

void mmp_destroy(struct mmpool * mp);

/* 0 for off, usually for debug
 * 1-6, the bigger of @param level, the more detailed */
void mmp_set_loglevel(int level);
