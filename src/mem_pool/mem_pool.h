/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2017/9/7
 *
 * a simple memory pool, for allocation of  fixed-size objects
 *
 * @history:
 * 2018/4/28 change to c
 */

#ifndef CP_MEM_POOL_H_
#define CP_MEM_POOL_H_

#define HP_HTTP_MAX 200

#ifdef __cplusplus
extern "C" {
#endif

struct mem_pool;

/* @param objsize > 0, or return NULL, @param factor [0-10] */
struct mem_pool * mp_create(int objsize, int factor);

void * mp_alloc(struct mem_pool * mp);

void mp_free(struct mem_pool * mp, void * p);
void mp_destroy(struct mem_pool * mp);

#ifdef __cplusplus
}
#endif

#endif /* CP_MEM_POOL_H_ */


