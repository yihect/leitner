#include "stdlib.h"
#include "memman_tests.h"


CuSuite* memmam_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_mem_init);
	SUITE_ADD_TEST(suite, test_mem_alloc);
	SUITE_ADD_TEST(suite, test_mem_combine);
	SUITE_ADD_TEST(suite, test_mem_free);
	return suite;
}

/****************************************/
/* 对memman模块中的各函数进行测试 */
extern struct mem_node *empty_list;
extern struct mem_node *busy_list; 

void test_mem_init(CuTest *tc)
{
	/* 开始的时候, empty_list 和 busy_list 都应该是空的. */
	CuAssertPtrEquals(tc, NULL, empty_list);
	CuAssertPtrEquals(tc, NULL, busy_list);


	/* 调用一次 mem_init. */
	mem_init();

	CuAssertPtrEquals(tc, NULL, busy_list);

	/* empty_list 这个时候肯定会指向一个mem_node节点, */ 
	/* 这个节点描述一段动态分配的内存 */
	CuAssertPtrNotNull(tc, empty_list);
	CuAssertPtrNotNull(tc, empty_list->start_addr);

	/* 我们必须确保被empty_list 指向的那个mem_node 必须*/
	/* 已经被正确初始化了. */
	CuAssertIntEquals(tc, BUF_POOL_LEN, empty_list->len);
	CuAssertPtrEquals(tc, NULL, empty_list->next);

	/* 为避免内存泄漏，我们通过手工的方式释放*/
	/* 在函数mem_init() 中动态分配的内存,这之后就结束*/
	/* 了对函数 mem_init() 进行的测试*/
	free(empty_list->start_addr);
	empty_list->start_addr = NULL;
	empty_list->next = NULL;
	empty_list->len = 0;
	free(empty_list);
	empty_list = NULL;
}

void test_mem_alloc(CuTest *tc)
{
	/* 为了在测试mem_alloc() 的时候不受mem_init() 函数的*/ 
	/* 干扰,我们在堆栈中分配内存和描述节点,并把*/
	/* 该节点插入到empty_list 中去. 我们这么做只是为 */
	/* 了单独测试mem_alloc() 函数，实际情况下如果你*/
	/* 能确保mem_init() 函数可以正常工作，你也可以不*/
	/* 这么做*/
	char buf[1000];
	struct mem_node mn = {buf, 1000, NULL};

	/* 我们把节点插入到empty_list 中去 */
	/* 就像mem_init() 函数做的那样,只不过那样是动态的*/ 
	empty_list = &mn;
	CuAssertPtrEquals(tc,NULL, busy_list);

	/* 我们调用mem_alloc() 继续测试 */
	char *buf1 = mem_alloc(100);

	/* asserting... */
	/* 第一次分配100长度之后，会修改原先*/
	/* 在empty_list 中的唯一节点 */
	CuAssertPtrEquals(tc,buf,buf1);
	CuAssertPtrEquals(tc,buf1+100, empty_list->start_addr);
	CuAssertIntEquals(tc,1000-100, empty_list->len);
	CuAssertPtrEquals(tc,NULL, empty_list->next);

	/* 第一次分配过程中，也会动态分配一个*/
	/* 新的节点，用来描述分配出来的那100长*/
	/* 的内存，这个节点会被插入到 busy_list 中去*/
	/* 我们也检查一下*/
	CuAssertPtrNotNull(tc, busy_list);
	CuAssertPtrEquals(tc, buf, busy_list->start_addr);
	CuAssertIntEquals(tc, 100, busy_list->len);
	CuAssertPtrEquals(tc, NULL, busy_list->next);
	CuAssertIntEquals(tc,1,node_num(busy_list));

	char *buf2 = mem_alloc(200);

	/* 第二次分配200长度之后，空闲内存会减小到*/
	/* 1000-100-200, 我们测试一下在empty_list 中的那个节点*/
	CuAssertPtrEquals(tc,buf2+200, empty_list->start_addr);
	CuAssertIntEquals(tc,1000-100-200, empty_list->len);
	CuAssertPtrEquals(tc,NULL, empty_list->next);

	/* 分配第2 个节点的时候，也会动态分配一个*/
	/* 新的节点,并把它插入到busy_list 中去，我们*/
	/* 仔细检查下*/
	CuAssertPtrNotNull(tc, busy_list);
	CuAssertPtrEquals(tc, buf+100, buf2);
	CuAssertPtrEquals(tc, buf+100, busy_list->start_addr);
	CuAssertIntEquals(tc, 200, busy_list->len);
	CuAssertPtrEquals(tc, NULL, busy_list->next->next);
	CuAssertPtrEquals(tc, buf1, busy_list->next->start_addr);
	CuAssertIntEquals(tc, 100, busy_list->next->len);
	CuAssertPtrEquals(tc, NULL, busy_list->next->next);
	CuAssertIntEquals(tc,2,node_num(busy_list));

	/* 第三次分配800的时候，由于pool中可用的内存*/
	/* 空间只有1000-100-200=700, 所以这次应该失败，*/
	/* 我们测试下返回值，应该为NULL ,如果要严格*/
	/* 一点，还要测试下empty_list 和 busy_list 是否没有*/
	/* 发生变化，这里我们省略不做了*/ 
	char *buf_no = mem_alloc(800);
	CuAssertPtrEquals(tc,NULL, buf_no);

	/* 最后一次分配700的时候，empty_list 中那个唯一*/
	/* 的节点就会被移动busy_list 中去，我们测试下*/
	char *buf4 = mem_alloc(700);
	CuAssertPtrEquals(tc,NULL,empty_list);
	CuAssertPtrEquals(tc,buf2+200,buf4);
	CuAssertPtrEquals(tc,buf4,busy_list->start_addr);
	CuAssertIntEquals(tc,700,busy_list->len);
	CuAssertPtrEquals(tc,busy_list->next->start_addr,buf2);
	CuAssertIntEquals(tc,3,node_num(busy_list));
	

	/* 最后,别忘记释放在mem_alloc() 中分配的动态内存 */
	/* 注意最后一次分配对应的节点实际上就是在函*/
	/* 数开头定义在堆栈中的那个节点mm ，所以这是*/
	/* 不用再另外释放的 */
	free(busy_list->next->next); /* 第一次分配的节点 */
	free(busy_list->next);	/* 第二次分配的节点 */

	busy_list = NULL;
	empty_list = NULL;
}


void init_mem_node(struct mem_node *pn, char *addr, 
						unsigned long len, struct mem_node *pnext)
{
	pn->start_addr = addr;
	pn->len = len;
	pn->next = pnext;
}


struct mem_node *find_node(struct mem_node *list, char *addr)
{
	struct mem_node *pn = list;

	while (pn != NULL)
	{
		if (pn->start_addr == addr)
			break;
		pn = pn->next;
	}

	return pn;
}

int node_num(struct mem_node *list)
{
	int n = 0;
	struct mem_node *pn = list;

	while (pn != NULL)
	{
		n++;
		pn = pn->next;
	}

	return n;
}

void test_mem_combine(CuTest *tc)
{
	/* mem_combile() 函数将把由参数给出的节点插入到*/
	/* empty_list 中去， 中间可能涉及到多种复杂的情况*/
	/* 为了更好的使测试做到单元化，我们先准备好*/
	/* 测试环境，即准备好empty_list 和busy_list 等两个链表*/
	/* 和malloc_alloc() 的测试一样，我们也用局部变量 */
	/* 因为mem_combine() 函数操作的是动态分配的节点, */
	/* 所以我们也需要动态分配测试节点 */
	char buf[1000];

	/* 节点位置*/
	/* 		100 +200							700 +50		800 +100			*/
  	/* 50 +20		300 +70	400 +60	600 +100		750 +50			900 +80 */

	/* 这些节点链到empty_list 中去 */
	struct mem_node *pn1 = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pn2 = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pn3 = (struct mem_node *)malloc(sizeof(struct mem_node));
	CuAssertPtrNotNull(tc,pn1);
	CuAssertPtrNotNull(tc,pn2);
	CuAssertPtrNotNull(tc,pn3);
	init_mem_node(pn1, buf+100, 200, NULL);
	init_mem_node(pn2, buf+700, 50, NULL);
	init_mem_node(pn3, buf+800, 100, NULL);

	/* 注意链到empty_list 中去的时候，按顺序排好队*/
	empty_list = pn1;
	pn1->next = pn2;
	pn2->next = pn3;
	CuAssertIntEquals(tc,3,node_num(empty_list));

	/* 这些节点将要插入到empty_list 中去 */
	struct mem_node *pna = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pnb = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pnc = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pnd = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pne = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pnf = (struct mem_node *)malloc(sizeof(struct mem_node));
	CuAssertPtrNotNull(tc,pna);
	CuAssertPtrNotNull(tc,pnb);
	CuAssertPtrNotNull(tc,pnc);
	CuAssertPtrNotNull(tc,pnd);
	CuAssertPtrNotNull(tc,pne);
	CuAssertPtrNotNull(tc,pnf);
	init_mem_node(pna,buf+50,20,NULL);
	init_mem_node(pnb,buf+300,70,NULL);
	init_mem_node(pnc,buf+400,60,NULL);
	init_mem_node(pnd,buf+600,100,NULL);
	init_mem_node(pne,buf+750,50,NULL);
	init_mem_node(pnf,buf+900,80,NULL);

	/* 插入pna */
	mem_combine(pna);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	CuAssertPtrEquals(tc,pna,empty_list);
	CuAssertPtrEquals(tc,pn1,empty_list->next);
	CuAssertPtrEquals(tc,buf+50,empty_list->start_addr);
	CuAssertIntEquals(tc,20,empty_list->len);
	CuAssertPtrEquals(tc,buf+100,empty_list->next->start_addr);
	CuAssertIntEquals(tc,200,empty_list->next->len);

	/* 插入pnb */
	mem_combine(pnb);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	struct mem_node *pn = find_node(empty_list, buf+100);
	CuAssertIntEquals(tc,270,pn->len);
	CuAssertPtrEquals(tc,pn2,pn->next);
	CuAssertPtrEquals(tc,buf+700,pn->next->start_addr);
	CuAssertIntEquals(tc,50,pn->next->len);

	/* 插入pnc */
	mem_combine(pnc);
	CuAssertIntEquals(tc,5,node_num(empty_list));
	CuAssertIntEquals(tc,270,pn->len);
	CuAssertPtrEquals(tc,pnc,pn->next);
	CuAssertPtrEquals(tc,pnc->start_addr,pn->next->start_addr);
	CuAssertIntEquals(tc,60,pn->next->len);
	CuAssertPtrEquals(tc,buf+700,pn->next->next->start_addr);
	CuAssertIntEquals(tc,50,pn->next->next->len);

	/* 插入pnd */
	mem_combine(pnd);
	CuAssertIntEquals(tc,5,node_num(empty_list));
	pn = find_node(empty_list, buf+400);
	CuAssertIntEquals(tc,60,pn->len);
	CuAssertPtrEquals(tc,buf+600,pn->next->start_addr);
	CuAssertIntEquals(tc,100+50,pn->next->len);

	/* 插入pne */
	mem_combine(pne);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	pn = pn->next;
	CuAssertPtrEquals(tc,buf+600,pn->start_addr);
	CuAssertIntEquals(tc,150+50+100,pn->len);

	/* 插入pnf */
	mem_combine(pnf);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	CuAssertPtrEquals(tc,buf+600,pn->start_addr);
	CuAssertIntEquals(tc,300+80,pn->len);

	/* 别忘记释放还未释放的动态内存 */
	/* 有部分节点已经在mem_combine() 函数内部释放掉*/
	pn = empty_list;
	while (pn != NULL)
	{
		free(pn);
		pn = pn->next;
	}

	/* 最后将两个list 置空,以期后面的使用能正确*/
	empty_list = NULL;
	busy_list = NULL;
}

void test_mem_free(CuTest *tc)
{
	/* 测试mem_free() 函数可以做的简单一点,因为*/
	/* 其他的函数都已经过测试，所以我们可以*/
	/* 直接拿来用，但是最好我们还是按照一定*/
	/* 的mem_alloc / mem_free 顺序来，以覆盖到程序*/
	/* 所有的执行路径*/

	/* 节点位置*/
	/* 		100 +200							700 +50		800 +100			*/
  	/* 50 +20		300 +70	400 +60	600 +100		750 +50			900 +80 */

	/* 分配50 */
	char *b1 = mem_alloc(50);

	/* 分配20 */
	char *b2 = mem_alloc(20);

	/* 分配230 */
	char *b3 = mem_alloc(230);

	/* 分配70 */
	char *b4 = mem_alloc(70);

	/* 分配90 */
	char *b5 = mem_alloc(90);

	/* 分配240 */
	char *b6 = mem_alloc(240);

	/* 分配50 */
	char *b7 = mem_alloc(50);

	/* 分配50 */
	char *b8 = mem_alloc(50);

	/* 分配100 */
	char *b9 = mem_alloc(100);

	/* 分配80 */
	char *b10 = mem_alloc(80);

	/* 释放b3 */
	mem_free(b3);
	CuAssertIntEquals(tc,2,node_num(empty_list));
	CuAssertPtrEquals(tc,b3,empty_list->start_addr);
	CuAssertIntEquals(tc,230,empty_list->len);
	CuAssertIntEquals(tc,9,node_num(busy_list));

	/* 释放b7 */
	mem_free(b7);
	CuAssertIntEquals(tc,3,node_num(empty_list));
	CuAssertPtrEquals(tc,b7,empty_list->next->start_addr);
	CuAssertIntEquals(tc,50,empty_list->next->len);
	CuAssertIntEquals(tc,8,node_num(busy_list));

	/* 释放b9 */
	mem_free(b9);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	CuAssertPtrEquals(tc,b9,empty_list->next->next->start_addr);
	CuAssertIntEquals(tc,100,empty_list->next->next->len);
	CuAssertIntEquals(tc,7,node_num(busy_list));

	/* 释放b1 */
	mem_free(b1);
	CuAssertIntEquals(tc,5,node_num(empty_list));
	CuAssertPtrEquals(tc,b1,empty_list->start_addr);
	CuAssertIntEquals(tc,50,empty_list->len);
	CuAssertIntEquals(tc,6,node_num(busy_list));
	CuAssertPtrEquals(tc,b10,busy_list->start_addr);
	CuAssertIntEquals(tc,80,busy_list->len);

	/* 先尝试destroy ,因为还有内存在使用中，*/
	/* 所以这个时候是不允许exit的 */
	mem_exit();

	/* 释放其他buf */
	mem_free(b2);
	mem_free(b4);
	mem_free(b5);
	mem_free(b6);
	mem_free(b8);
	mem_free(b10);


	/* exit */
	mem_exit();
}









