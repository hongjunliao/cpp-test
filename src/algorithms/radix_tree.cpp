/*!
 * test radix_tree, from book <Introduction to Algorithms, 3th Edition>_12-2
 * @author hongjun.liao <docici@126.com>, @date 2017//04/29
 *
 * sort using radix_tree:
 * sort "1011, 10, 011, 100, 0" TO "0, 011, 10, 100, 1011"
 */

#include <bitset>	/* std::bitset */
#include <limits.h>

struct radix_tree_node;

#define RTR_KEY(str) (std::bitset<sizeof(size_t) * 8>(str).to_ulong())
#define RTR_KEY_TO_STR(key) (std::bitset<sizeof(size_t) * 8>(key).to_string().c_str())
#define RTR_POOL_N (32 * 1024 * 1024 / sizeof(radix_tree_node))

struct radix_tree_node_pool {
	radix_tree_node * n[32];
	size_t i;
	size_t j;
};

static struct radix_tree_node_pool g_pool = { 0 };

struct radix_tree_node {
	radix_tree_node * left;     /* left child */
	radix_tree_node * right;    /* right child */
	radix_tree_node * p;	    /* parent */

	bool on;	/* true is this node has value */

	int key;

};

radix_tree_node * radix_tree_node_get(radix_tree_node_pool & p)
{
//	fprintf(stdout, "%s:\n", __FUNCTION__);
	if(p.i == 32){
		fprintf(stderr, "%s: outof memory\n", __FUNCTION__);
		return 0;
	}
	if(p.i == 0 || p.j == RTR_POOL_N)
		p.n[p.i++] = (radix_tree_node * )calloc(RTR_POOL_N, sizeof(radix_tree_node));

	if(p.j == RTR_POOL_N)
		p.j = 0;

	auto ret = &p.n[p.i - 1][p.j++];
	ret->on = false;

//	fprintf(stdout, "%s:\n", __FUNCTION__);
	return ret;
}

int radix_tree_insert(radix_tree_node *& root, size_t key)
{
	fprintf(stdout, "%s: key=%s\n", __FUNCTION__, RTR_KEY_TO_STR(key));

	std::bitset<sizeof(size_t)> bits(key), bitn;

	if(!root)
		root = radix_tree_node_get(g_pool);

	auto x = root;
	for(size_t i = 0; i < bits.size(); ++i){
		if(bits[i]){
			if(!x->right)
				x->right = radix_tree_node_get(g_pool);
			bitn.set(i);

			x = x->right;
		}
		else{
			if(!x->left)
				x->left = radix_tree_node_get(g_pool);

			bitn.set(i, false);

			x = x->left;
		}

		if(bitn.to_ulong() == key){
			x->key = bitn.to_ulong();
			x->on = true;
			return 0;
		}
	}
//	fprintf(stdout, "%s:\n", __FUNCTION__);

	return -1;
}

static void radix_tree_inorder_walk(radix_tree_node const * root)
{
	if(!root)
		return;

	radix_tree_inorder_walk(root->left);
	if(root->on)
		fprintf(stdout, "%s, ", RTR_KEY_TO_STR(root->key));
	radix_tree_inorder_walk(root->right);
}

int radix_tree_sort(size_t const * keys, size_t n, size_t * out)
{
	radix_tree_node * rtr = 0;

	for(size_t i = 0; i < n; ++i){
		radix_tree_insert(rtr, keys[i]);
	}

}

int test_radix_tree_main(int argc, char ** argv)
{
	size_t keys[] = { RTR_KEY("1011"), RTR_KEY("10"), RTR_KEY("011"),
			RTR_KEY("100"), RTR_KEY("0") };
	auto KEYS_SZ = sizeof(keys) / sizeof(keys[0]);

	size_t out[KEYS_SZ];

	radix_tree_sort(keys, KEYS_SZ, out);

	return -1;
}
