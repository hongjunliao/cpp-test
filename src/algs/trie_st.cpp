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

struct trie_st_node{
	int val;
	trie_st_node * next;
};

static trie_st_node * trie_st_node_alloc(size_t r)
{
	auto ret = (trie_st_node *)calloc(r, sizeof(trie_st_node));
	return ret;
}

static int trie_st_do_put(trie_st_node & tr, char const * key, size_t len , int val)
{
	if(len == 0){
		tr.val = val;
		return 0;
	}
	if(!tr.next)
		tr.next = trie_st_node_alloc(R);

	return trie_st_do_put(tr.next[key[0]], key + 1, len - 1, val);
}

static int trie_st_do_get(trie_st_node const & tr, char const * key, size_t len, int & val)
{
	if(len == 0){
		val = tr.val;
		return 0;
	}
	if(!tr.next)
		return -1;
	return trie_st_do_get(tr.next[key[0]], key + 1, len - 1, val);
}

static int trie_st_put(trie_st_node & tr, char const * key, int val)
{
	if(!(key && key[0] != '\0'))
		return -1;
	return trie_st_do_put(tr, key, strlen(key), val);
}

static int trie_st_get(trie_st_node const & tr, char const * key, int & val)
{
	if(!(key && key[0] != '\0'))
		return -1;
	return trie_st_do_get(tr, key, strlen(key), val);
}

int test_trie_st_main_1(int argc, char ** argv)
{
	std::pair<char const *, int> keys[] = { {"she", 1}, {"sells", 2},
			{"sea", 3}, {"shells", 4}, {"by", 5}, {"the", 666}, {"sea", 7}, {"shore", 8} };
	trie_st_node tr{ 0 };

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/* 三向单词查找树:
 * 每个结点(含根结点)都有一个字符,三条链接和一个值;
 * 这三条链接分别对应着当前字母小于,等于,大于结点字母的所有键
 */
struct tst_node{
	char c;
	int val;
	tst_node * next[3];
};

static tst_node * gnodes = 0;
static size_t ginodes = 0;

static int tst_node_get( tst_node * node)
{
	size_t N = 1024 * 1024 / sizeof(tst_node);
	for(;;){
		 if(!gnodes){
			 gnodes = (tst_node *)calloc(N, sizeof(tst_node));
			 if(!gnodes){
				 fprintf(stderr, "%s: calloc failed\n", __FUNCTION__);
				 exit(0);
			 }
		 }
		 /* FIXME: memory leak!!! ugly but worked */
		 if(ginodes == N - 1){
			 gnodes = 0;
			 ginodes = 0;
			 continue;
		 }
		 node = &gnodes[ginodes++];
		 return 0;
	}
}

static int tst_do_put(tst_node *& tr, char const * key, size_t len, int val)
{
	char c = key[0];
	if(!tr){
		auto r = tst_node_get(tr);
		if(r != 0) return -1;
		tr->c = c;
	}

	if(c < tr->c)
		return tst_do_put(tr->next[0], key, len, val);
	else if(c > tr->c)
		return tst_do_put(tr->next[2], key, len, val);
	else if(len > 1)
		return tst_do_put(tr->next[1], key + 1, len - 1, val);

	tr->val = val;
	return 0;
}

static int tst_put(tst_node *& tr, char const * key, int val)
{
	if(!(key && key[0] != '\0'))
		return -1;
	return tst_do_put(tr, key, strlen(key), val);
}

static int tst_do_get(tst_node *& tr, char const * key, size_t len, int & val)
{
	if(!tr) return -1;

	char c = key[0];
	if(c < tr->c)
		return tst_do_get(tr->next[0], key, len, val);
	else if(c > tr->c)
		return tst_do_get(tr->next[2], key, len, val);
	else if(len > 1)
		return tst_do_get(tr->next[1], key + 1, len - 1, val);

	val = tr->val;
	return 0;
}

static int tst_get(tst_node *& tr, char const * key, int & val)
{
	if(!(key && key[0] != '\0'))
		return -1;
	return tst_do_get(tr, key, strlen(key), val);
}

int test_tst_main(int argc, char ** argv)
{
	std::pair<char const *, int> keys[] = { {"she", 1}, {"sells", 2},
			{"sea", 3}, {"shells", 4}, {"by", 5}, {"the", 666}, {"sea", 7}, {"shore", 8} };
	tst_node * tr = 0;

	for(auto const& item : keys){
		auto r = tst_put(tr, item.first, item.second);
		if(r != 0)
			fprintf(stderr, "%s: tst_put failed for <%s,%d>\n", __FUNCTION__, item.first, item.second);
	}

	auto key = "the";
	int val;
	auto r = tst_get(tr, key, val);
	if(r != 0){
		fprintf(stderr, "%s: tst_get failed for '%s'\n", __FUNCTION__, key);
		return -1;
	}
	fprintf(stdout, "%s: found for key='%s', value=%d\n", __FUNCTION__, key, val);

	key = "that";
	r = tst_get(tr, key, val);
	if(r != 0){
		fprintf(stderr, "%s: tst_get failed for '%s'\n", __FUNCTION__, key);
		return -1;
	}
	fprintf(stdout, "%s: found for key='%s', value=%d\n", __FUNCTION__, key, val);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_trie_st_main(int argc, char ** argv)
{
	return test_tst_main(argc, argv);
	return test_trie_st_main_1(argc, argv);
}
