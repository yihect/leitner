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
/* ��memmanģ���еĸ��������в��� */
extern struct mem_node *empty_list;
extern struct mem_node *busy_list; 

void test_mem_init(CuTest *tc)
{
	/* ��ʼ��ʱ��, empty_list �� busy_list ��Ӧ���ǿյ�. */
	CuAssertPtrEquals(tc, NULL, empty_list);
	CuAssertPtrEquals(tc, NULL, busy_list);


	/* ����һ�� mem_init. */
	mem_init();

	CuAssertPtrEquals(tc, NULL, busy_list);

	/* empty_list ���ʱ��϶���ָ��һ��mem_node�ڵ�, */ 
	/* ����ڵ�����һ�ζ�̬������ڴ� */
	CuAssertPtrNotNull(tc, empty_list);
	CuAssertPtrNotNull(tc, empty_list->start_addr);

	/* ���Ǳ���ȷ����empty_list ָ����Ǹ�mem_node ����*/
	/* �Ѿ�����ȷ��ʼ����. */
	CuAssertIntEquals(tc, BUF_POOL_LEN, empty_list->len);
	CuAssertPtrEquals(tc, NULL, empty_list->next);

	/* Ϊ�����ڴ�й©������ͨ���ֹ��ķ�ʽ�ͷ�*/
	/* �ں���mem_init() �ж�̬������ڴ�,��֮��ͽ���*/
	/* �˶Ժ��� mem_init() ���еĲ���*/
	free(empty_list->start_addr);
	empty_list->start_addr = NULL;
	empty_list->next = NULL;
	empty_list->len = 0;
	free(empty_list);
	empty_list = NULL;
}

void test_mem_alloc(CuTest *tc)
{
	/* Ϊ���ڲ���mem_alloc() ��ʱ����mem_init() ������*/ 
	/* ����,�����ڶ�ջ�з����ڴ�������ڵ�,����*/
	/* �ýڵ���뵽empty_list ��ȥ. ������ô��ֻ��Ϊ */
	/* �˵�������mem_alloc() ������ʵ������������*/
	/* ��ȷ��mem_init() ��������������������Ҳ���Բ�*/
	/* ��ô��*/
	char buf[1000];
	struct mem_node mn = {buf, 1000, NULL};

	/* ���ǰѽڵ���뵽empty_list ��ȥ */
	/* ����mem_init() ������������,ֻ���������Ƕ�̬��*/ 
	empty_list = &mn;
	CuAssertPtrEquals(tc,NULL, busy_list);

	/* ���ǵ���mem_alloc() �������� */
	char *buf1 = mem_alloc(100);

	/* asserting... */
	/* ��һ�η���100����֮�󣬻��޸�ԭ��*/
	/* ��empty_list �е�Ψһ�ڵ� */
	CuAssertPtrEquals(tc,buf,buf1);
	CuAssertPtrEquals(tc,buf1+100, empty_list->start_addr);
	CuAssertIntEquals(tc,1000-100, empty_list->len);
	CuAssertPtrEquals(tc,NULL, empty_list->next);

	/* ��һ�η�������У�Ҳ�ᶯ̬����һ��*/
	/* �µĽڵ㣬�������������������100��*/
	/* ���ڴ棬����ڵ�ᱻ���뵽 busy_list ��ȥ*/
	/* ����Ҳ���һ��*/
	CuAssertPtrNotNull(tc, busy_list);
	CuAssertPtrEquals(tc, buf, busy_list->start_addr);
	CuAssertIntEquals(tc, 100, busy_list->len);
	CuAssertPtrEquals(tc, NULL, busy_list->next);
	CuAssertIntEquals(tc,1,node_num(busy_list));

	char *buf2 = mem_alloc(200);

	/* �ڶ��η���200����֮�󣬿����ڴ���С��*/
	/* 1000-100-200, ���ǲ���һ����empty_list �е��Ǹ��ڵ�*/
	CuAssertPtrEquals(tc,buf2+200, empty_list->start_addr);
	CuAssertIntEquals(tc,1000-100-200, empty_list->len);
	CuAssertPtrEquals(tc,NULL, empty_list->next);

	/* �����2 ���ڵ��ʱ��Ҳ�ᶯ̬����һ��*/
	/* �µĽڵ�,���������뵽busy_list ��ȥ������*/
	/* ��ϸ�����*/
	CuAssertPtrNotNull(tc, busy_list);
	CuAssertPtrEquals(tc, buf+100, buf2);
	CuAssertPtrEquals(tc, buf+100, busy_list->start_addr);
	CuAssertIntEquals(tc, 200, busy_list->len);
	CuAssertPtrEquals(tc, NULL, busy_list->next->next);
	CuAssertPtrEquals(tc, buf1, busy_list->next->start_addr);
	CuAssertIntEquals(tc, 100, busy_list->next->len);
	CuAssertPtrEquals(tc, NULL, busy_list->next->next);
	CuAssertIntEquals(tc,2,node_num(busy_list));

	/* �����η���800��ʱ������pool�п��õ��ڴ�*/
	/* �ռ�ֻ��1000-100-200=700, �������Ӧ��ʧ�ܣ�*/
	/* ���ǲ����·���ֵ��Ӧ��ΪNULL ,���Ҫ�ϸ�*/
	/* һ�㣬��Ҫ������empty_list �� busy_list �Ƿ�û��*/
	/* �����仯����������ʡ�Բ�����*/ 
	char *buf_no = mem_alloc(800);
	CuAssertPtrEquals(tc,NULL, buf_no);

	/* ���һ�η���700��ʱ��empty_list ���Ǹ�Ψһ*/
	/* �Ľڵ�ͻᱻ�ƶ�busy_list ��ȥ�����ǲ�����*/
	char *buf4 = mem_alloc(700);
	CuAssertPtrEquals(tc,NULL,empty_list);
	CuAssertPtrEquals(tc,buf2+200,buf4);
	CuAssertPtrEquals(tc,buf4,busy_list->start_addr);
	CuAssertIntEquals(tc,700,busy_list->len);
	CuAssertPtrEquals(tc,busy_list->next->start_addr,buf2);
	CuAssertIntEquals(tc,3,node_num(busy_list));
	

	/* ���,�������ͷ���mem_alloc() �з���Ķ�̬�ڴ� */
	/* ע�����һ�η����Ӧ�Ľڵ�ʵ���Ͼ����ں�*/
	/* ����ͷ�����ڶ�ջ�е��Ǹ��ڵ�mm ����������*/
	/* �����������ͷŵ� */
	free(busy_list->next->next); /* ��һ�η���Ľڵ� */
	free(busy_list->next);	/* �ڶ��η���Ľڵ� */

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
	/* mem_combile() ���������ɲ��������Ľڵ���뵽*/
	/* empty_list ��ȥ�� �м�����漰�����ָ��ӵ����*/
	/* Ϊ�˸��õ�ʹ����������Ԫ����������׼����*/
	/* ���Ի�������׼����empty_list ��busy_list ����������*/
	/* ��malloc_alloc() �Ĳ���һ��������Ҳ�þֲ����� */
	/* ��Ϊmem_combine() �����������Ƕ�̬����Ľڵ�, */
	/* ��������Ҳ��Ҫ��̬������Խڵ� */
	char buf[1000];

	/* �ڵ�λ��*/
	/* 		100 +200							700 +50		800 +100			*/
  	/* 50 +20		300 +70	400 +60	600 +100		750 +50			900 +80 */

	/* ��Щ�ڵ�����empty_list ��ȥ */
	struct mem_node *pn1 = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pn2 = (struct mem_node *)malloc(sizeof(struct mem_node));
	struct mem_node *pn3 = (struct mem_node *)malloc(sizeof(struct mem_node));
	CuAssertPtrNotNull(tc,pn1);
	CuAssertPtrNotNull(tc,pn2);
	CuAssertPtrNotNull(tc,pn3);
	init_mem_node(pn1, buf+100, 200, NULL);
	init_mem_node(pn2, buf+700, 50, NULL);
	init_mem_node(pn3, buf+800, 100, NULL);

	/* ע������empty_list ��ȥ��ʱ�򣬰�˳���źö�*/
	empty_list = pn1;
	pn1->next = pn2;
	pn2->next = pn3;
	CuAssertIntEquals(tc,3,node_num(empty_list));

	/* ��Щ�ڵ㽫Ҫ���뵽empty_list ��ȥ */
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

	/* ����pna */
	mem_combine(pna);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	CuAssertPtrEquals(tc,pna,empty_list);
	CuAssertPtrEquals(tc,pn1,empty_list->next);
	CuAssertPtrEquals(tc,buf+50,empty_list->start_addr);
	CuAssertIntEquals(tc,20,empty_list->len);
	CuAssertPtrEquals(tc,buf+100,empty_list->next->start_addr);
	CuAssertIntEquals(tc,200,empty_list->next->len);

	/* ����pnb */
	mem_combine(pnb);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	struct mem_node *pn = find_node(empty_list, buf+100);
	CuAssertIntEquals(tc,270,pn->len);
	CuAssertPtrEquals(tc,pn2,pn->next);
	CuAssertPtrEquals(tc,buf+700,pn->next->start_addr);
	CuAssertIntEquals(tc,50,pn->next->len);

	/* ����pnc */
	mem_combine(pnc);
	CuAssertIntEquals(tc,5,node_num(empty_list));
	CuAssertIntEquals(tc,270,pn->len);
	CuAssertPtrEquals(tc,pnc,pn->next);
	CuAssertPtrEquals(tc,pnc->start_addr,pn->next->start_addr);
	CuAssertIntEquals(tc,60,pn->next->len);
	CuAssertPtrEquals(tc,buf+700,pn->next->next->start_addr);
	CuAssertIntEquals(tc,50,pn->next->next->len);

	/* ����pnd */
	mem_combine(pnd);
	CuAssertIntEquals(tc,5,node_num(empty_list));
	pn = find_node(empty_list, buf+400);
	CuAssertIntEquals(tc,60,pn->len);
	CuAssertPtrEquals(tc,buf+600,pn->next->start_addr);
	CuAssertIntEquals(tc,100+50,pn->next->len);

	/* ����pne */
	mem_combine(pne);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	pn = pn->next;
	CuAssertPtrEquals(tc,buf+600,pn->start_addr);
	CuAssertIntEquals(tc,150+50+100,pn->len);

	/* ����pnf */
	mem_combine(pnf);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	CuAssertPtrEquals(tc,buf+600,pn->start_addr);
	CuAssertIntEquals(tc,300+80,pn->len);

	/* �������ͷŻ�δ�ͷŵĶ�̬�ڴ� */
	/* �в��ֽڵ��Ѿ���mem_combine() �����ڲ��ͷŵ�*/
	pn = empty_list;
	while (pn != NULL)
	{
		free(pn);
		pn = pn->next;
	}

	/* �������list �ÿ�,���ں����ʹ������ȷ*/
	empty_list = NULL;
	busy_list = NULL;
}

void test_mem_free(CuTest *tc)
{
	/* ����mem_free() �����������ļ�һ��,��Ϊ*/
	/* �����ĺ������Ѿ������ԣ��������ǿ���*/
	/* ֱ�������ã�����������ǻ��ǰ���һ��*/
	/* ��mem_alloc / mem_free ˳�������Ը��ǵ�����*/
	/* ���е�ִ��·��*/

	/* �ڵ�λ��*/
	/* 		100 +200							700 +50		800 +100			*/
  	/* 50 +20		300 +70	400 +60	600 +100		750 +50			900 +80 */

	/* ����50 */
	char *b1 = mem_alloc(50);

	/* ����20 */
	char *b2 = mem_alloc(20);

	/* ����230 */
	char *b3 = mem_alloc(230);

	/* ����70 */
	char *b4 = mem_alloc(70);

	/* ����90 */
	char *b5 = mem_alloc(90);

	/* ����240 */
	char *b6 = mem_alloc(240);

	/* ����50 */
	char *b7 = mem_alloc(50);

	/* ����50 */
	char *b8 = mem_alloc(50);

	/* ����100 */
	char *b9 = mem_alloc(100);

	/* ����80 */
	char *b10 = mem_alloc(80);

	/* �ͷ�b3 */
	mem_free(b3);
	CuAssertIntEquals(tc,2,node_num(empty_list));
	CuAssertPtrEquals(tc,b3,empty_list->start_addr);
	CuAssertIntEquals(tc,230,empty_list->len);
	CuAssertIntEquals(tc,9,node_num(busy_list));

	/* �ͷ�b7 */
	mem_free(b7);
	CuAssertIntEquals(tc,3,node_num(empty_list));
	CuAssertPtrEquals(tc,b7,empty_list->next->start_addr);
	CuAssertIntEquals(tc,50,empty_list->next->len);
	CuAssertIntEquals(tc,8,node_num(busy_list));

	/* �ͷ�b9 */
	mem_free(b9);
	CuAssertIntEquals(tc,4,node_num(empty_list));
	CuAssertPtrEquals(tc,b9,empty_list->next->next->start_addr);
	CuAssertIntEquals(tc,100,empty_list->next->next->len);
	CuAssertIntEquals(tc,7,node_num(busy_list));

	/* �ͷ�b1 */
	mem_free(b1);
	CuAssertIntEquals(tc,5,node_num(empty_list));
	CuAssertPtrEquals(tc,b1,empty_list->start_addr);
	CuAssertIntEquals(tc,50,empty_list->len);
	CuAssertIntEquals(tc,6,node_num(busy_list));
	CuAssertPtrEquals(tc,b10,busy_list->start_addr);
	CuAssertIntEquals(tc,80,busy_list->len);

	/* �ȳ���destroy ,��Ϊ�����ڴ���ʹ���У�*/
	/* �������ʱ���ǲ�����exit�� */
	mem_exit();

	/* �ͷ�����buf */
	mem_free(b2);
	mem_free(b4);
	mem_free(b5);
	mem_free(b6);
	mem_free(b8);
	mem_free(b10);


	/* exit */
	mem_exit();
}









