/*!
 * a simple memory pool, for allocation of  lots of small objects
 * @author hongjun.liao <docici@126.com>, @date 2017/9/7
 */

struct mem_pool;

mem_pool * mp_create(int objsize);

void * mp_alloc(mem_pool * mp);
#define mp_alloct(T, mp) ((T *)mp_alloc(mp))

void mp_free(void * p);
void mp_destroy(mem_pool * mp);
