#include "CuTest.h"
#include "enum_hash_tests.h"
#include "list.h"
#include "rbtree.h"
#include "objpool.h"

CuSuite* enum_hash_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_enum_hash);
	return suite;
}

struct hashnode;
struct enumnode {
	struct list_head en_node;
	struct list_head lnode;
	struct hashnode *hnp;
	char *enum_name;
	unsigned int enum_val;
};

struct hashnode {
	struct rb_node node;
	unsigned int hash;	/* hash value */
	unsigned int cnt;	/* cnt of enum with same value */
	struct list_head lhead;
};

static struct objpool *hashnode_cache;
static struct objpool *enumnode_cache;

static struct list_head en_list;	/* list of all enumnode */

struct hashnode *htree_search(struct rb_root *root, unsigned hval)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct hashnode *data = container_of(node, struct hashnode, node);

		if (hval < data->hash)
			node = node->rb_left;
		else if (hval > data->hash)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

struct hashnode *htree_insert(struct rb_root *root, unsigned hval, unsigned eval, char *ename)
{
	struct hashnode *data = NULL;
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct hashnode *this = container_of(*new, struct hashnode, node);
		int result = hval - this->hash;

		parent = *new;
		if (result < 0)
			new = &((*new)->rb_left);
		else if (result > 0)
			new = &((*new)->rb_right);
		else {
			//printf("hval: %ld, ename: %s, this->hash: %ld \n", hval, ename, this->hash);
			data = this;
			break;
		}
	}

	struct enumnode *pen = objpool_zalloc(enumnode_cache);
	pen->enum_name = ename;
	pen->enum_val = eval;

	/* add the pen to enumnode list of the saved hashnode if there has been
	 * one in the rbtree, or create a new hashnode with the pen, and then
	 * add the newly created hasnnode to the rbtree */
	if (data != NULL) {
		list_add(&pen->lnode, &data->lhead);
		data->cnt++;
		list_add_tail(&pen->en_node, &en_list);
		pen->hnp = data;
		return data;
	} else {
		data = objpool_zalloc(hashnode_cache);
		INIT_LIST_HEAD(&data->lhead);
		data->hash = hval;
		list_add(&pen->lnode, &data->lhead);
		data->cnt++;
		list_add_tail(&pen->en_node, &en_list);
		pen->hnp = data;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, root);

	return data;
}

/* macro for preparing hash_arg
 * l: line contain the heading string
 * s: heading string
 * sl: len of heading string
 */
#define prepare_hash_arg(l, s, sl)	\
		if (!strncmp(l, #s, sl))	ha.from = sl;

void test_enum_hash(CuTest *tc)
{
	INIT_LIST_HEAD(&en_list);
	hashnode_cache = create_objpool(sizeof(struct hashnode), 0, NULL);
	enumnode_cache = create_objpool(sizeof(struct enumnode), 0, NULL);

	struct rb_root hashtree = RB_ROOT;
	FILE *fp = fopen("../conf/enum.txt", "r");
	CuAssertPtrNotNull(tc, fp);

	char *line = NULL, *buf = NULL;
	size_t len = 0;
	struct hash_arg ha = {0, 6, 'A', 26, "_-+=", UINT32_MAX}; \
	while(file_getline(&buf, &len, fp) != -1) {
		line = clean_line(dupstr(buf));
		if (line[0] == '\0')
			continue;

		/* delete the final comma */
		line[strlen(line)-1] = '\0';

		/* preparing hash arg */
		prepare_hash_arg(line, BDT_, 4);
		prepare_hash_arg(line, WT_, 3);
		prepare_hash_arg(line, TS_PT_, 6);
		prepare_hash_arg(line, TS_PG_, 6);
		prepare_hash_arg(line, TS_RF_, 6);
		prepare_hash_arg(line, TS_V_, 5);
		prepare_hash_arg(line, TS_G_, 5);
		prepare_hash_arg(line, TS_S_, 5);
		prepare_hash_arg(line, UW_PT_, 6);
		prepare_hash_arg(line, UW_PG_, 6);
		prepare_hash_arg(line, UW_RF_, 6);
		prepare_hash_arg(line, UW_VV_, 6);
		prepare_hash_arg(line, UW_GG_, 6);
		prepare_hash_arg(line, UW_XX_, 6);
		prepare_hash_arg(line, GLT_, 4);

		unsigned hash = str_hash(line, &ha);
		htree_insert(&hashtree, hash, 0, line);
	}

	/* check uniqueness of mapping from hashvalue to enum name */
	struct rb_node *n = NULL;
	struct enumnode *en = NULL;
	unsigned i = 0;
	for (n = rb_first(&hashtree); n != NULL; n = rb_next(n)) {
		struct hashnode *node = rb_entry(n, struct hashnode, node);
		if (node->cnt > 1) {
			printf("%ld hashed of enums: \n", node->hash);
			list_for_each_entry(en, &node->lhead, lnode) {
				printf("	%s\n", en->enum_name);
			}
		}
	}

	/* output enum declare content like:
	 * XX(BDT_PT, ,409)	\
	 */
	FILE *fpw = fopen("../conf/enum_content.txt", "w+");
	CuAssertPtrNotNull(tc, fpw);
	en = NULL;
	list_for_each_entry(en, &en_list, en_node) {
		fprintf(fpw, "	XX(%s, , %ld)	\\\n", en->enum_name, en->hnp->hash);
	}


	/* do clean work */
	fclose(fpw);
	fclose(fp);
	free(buf);

	destroy_objpool(hashnode_cache);
	destroy_objpool(enumnode_cache);
}




