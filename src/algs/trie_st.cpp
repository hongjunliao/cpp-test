/*!
 * This file is Part of cpp-test
 * @author hongjun.liao <docici@126.com>, @date 2017//06/01
 *
 * 符串处理算法--单词查找树
 *             ________(root)___
 *             b    ____s_      t
 *             y  _e_   __h__   h
 *               a   l  e    o  e
 *                   l  l    r
 *                   s  l    e
 *                      s
 */
#include <stdio.h>
#include <stdlib.h>    /* calloc */
#include <string.h>    /* strlen */
#include <utility>     /* std::pair */

#define R 256  /* Alphabets */

struct tst_node{
	int val;
	tst_node * next;
};

static tst_node * tst_node_alloc(size_t r)
{
	auto ret = (tst_node *)calloc(r, sizeof(tst_node));
	return ret;
}

static int trie_st_do_put(tst_node & tr, char const * key, size_t len , int val)
{
	if(len == 0){
		tr.val = val;
		return 0;
	}
	if(!tr.next)
		tr.next = tst_node_alloc(R);

	return trie_st_do_put(tr.next[key[0]], key + 1, len - 1, val);
}

static int trie_st_do_get(tst_node const & tr, char const * key, size_t len, int & val)
{
	if(len == 0){
		val = tr.val;
		return 0;
	}
	if(!tr.next)
		return -1;
	return trie_st_do_get(tr.next[key[0]], key + 1, len - 1, val);
}

static int trie_st_put(tst_node & tr, char const * key, int val)
{
	if(!(key && key[0] != '\0'))
		return -1;
	return trie_st_do_put(tr, key, strlen(key), val);
}

static int trie_st_get(tst_node const & tr, char const * key, int & val)
{
	if(!(key && key[0] != '\0'))
		return -1;
	return trie_st_do_get(tr, key, strlen(key), val);
}

int test_trie_st_main(int argc, char ** argv)
{
	std::pair<char const *, int> keys[] = { {"she", 1}, {"sells", 2},
			{"sea", 3}, {"shells", 4}, {"by", 5}, {"the", 666}, {"sea", 7}, {"shore", 8} };
	tst_node tr{ 0 };

	for(auto const& item : keys){
		auto r = trie_st_put(tr, item.first, item.second);
		if(r != 0)
			fprintf(stderr, "%s: trie_st_put failed for <%s,%d>\n", __FUNCTION__, item.first, item.second);
	}

	auto key = "the";
	int val;
	auto r = trie_st_get(tr, key, val);
	if(r != 0){
		fprintf(stderr, "%s: trie_st_get failed for '%s'\n", __FUNCTION__, key);
		return -1;
	}
	fprintf(stdout, "%s: found for key='%s', value=%d\n", __FUNCTION__, key, val);

	key = "that";
	r = trie_st_get(tr, key, val);
	if(r != 0){
		fprintf(stderr, "%s: trie_st_get failed for '%s'\n", __FUNCTION__, key);
		return -1;
	}
	fprintf(stdout, "%s: found for key='%s', value=%d\n", __FUNCTION__, key, val);

	return 0;
}
