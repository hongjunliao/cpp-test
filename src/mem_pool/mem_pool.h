/*!
 * a simple memory pool, for allocation of  lots of small objects
 * @author hongjun.liao <docici@126.com>, @date 2017/9/7
 */

struct mem_pool;

/* @param objsize > 0, or return NULL, @param factor [0-10] */
mem_pool * mp_create(int objsize, int factor);

void * mp_alloc(mem_pool * mp);
#define mp_alloct(mp, T) ((T *)mp_alloc(mp))

void mp_free(void * p);
void mp_destroy(mem_pool * mp);

/* usually for debug, 1-6, the bigger of @param level, the more detailed */
void mp_set_loglevel(int level);
