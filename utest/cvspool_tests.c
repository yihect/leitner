#include "CuTest.h"
#include "cvspool.h"
#include "cvspool_tests.h"
#include "util.h"

CuSuite* cvspool_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_fuck_macro);
	SUITE_ADD_TEST(suite, test_slots_num_adjust);
	SUITE_ADD_TEST(suite, test_init_cvspool);
	SUITE_ADD_TEST(suite, test_alloc1_cvspool);
	SUITE_ADD_TEST(suite, test_alloc2_cvspool);
	SUITE_ADD_TEST(suite, test_free_cvspool);
	return suite;
}

void test_fuck_macro(CuTest *tc)
{
	u_int8_t n8 = 12;
	u_int16_t n16 = 34;
	u_int32_t n32 = 56;

	int remained = 1024;
	u_int16_t min = MIN((MAX_BLK_SIZE>>2), (remained>>2));

	CuAssertIntEquals(tc, 0, IBOFF_OF_SLOTABS(0));
	CuAssertIntEquals(tc, 1, IBOFF_OF_SLOTABS(1));
	CuAssertIntEquals(tc, 65530, IBOFF_OF_SLOTABS(65530));
	CuAssertIntEquals(tc, 65535, IBOFF_OF_SLOTABS(65535));
	CuAssertIntEquals(tc, 0, IBOFF_OF_SLOTABS(65536));

	CuAssertIntEquals(tc, 0, BLKIDX_OF_SLOTABS(0));
	CuAssertIntEquals(tc, 0, BLKIDX_OF_SLOTABS(65530));
	CuAssertIntEquals(tc, 0, BLKIDX_OF_SLOTABS(65535));
	CuAssertIntEquals(tc, 1, BLKIDX_OF_SLOTABS(65536));
	CuAssertIntEquals(tc, 1, BLKIDX_OF_SLOTABS(131071));
	CuAssertIntEquals(tc, 2, BLKIDX_OF_SLOTABS(131072));

	CuAssertIntEquals(tc, 1, PNBLKIDX(0,1));
	CuAssertIntEquals(tc, 2, PNBLKIDX(0,2));
	CuAssertIntEquals(tc, 8, PNBLKIDX(0,8));
	CuAssertIntEquals(tc, 0x10, PNBLKIDX(1,0));
	CuAssertIntEquals(tc, 0x20, PNBLKIDX(2,0));
	CuAssertIntEquals(tc, 0x80, PNBLKIDX(8,0));

	CuAssertIntEquals(tc, 0, SLOTABS_POS(0,0));
	CuAssertIntEquals(tc, 65536, SLOTABS_POS(1,0));
	CuAssertIntEquals(tc, 131072, SLOTABS_POS(2,0));
	CuAssertIntEquals(tc, 1048575, SLOTABS_POS(0xf,0xffff));
}

void test_slots_num_adjust(CuTest *tc)
{
	printf("test_slots_num_adjust() testing... \n");

	/* make sure there are 3 slots at least */
	CuAssertIntEquals(tc, 3, slots_num_adjust(1));
	CuAssertIntEquals(tc, 3, slots_num_adjust(2));

	/* across block border: make sure after blocked(+3) in one trunk,
	 * the last block has enouth slots space */
	CuAssertIntEquals(tc, 65533, slots_num_adjust(65533));
	CuAssertIntEquals(tc, 65534+2, slots_num_adjust(65534));
	CuAssertIntEquals(tc, 65535+1, slots_num_adjust(65535));
	CuAssertIntEquals(tc, 65536, slots_num_adjust(65536));
	CuAssertIntEquals(tc, 65537, slots_num_adjust(65537));
	CuAssertIntEquals(tc, 65538, slots_num_adjust(65538));
	CuAssertIntEquals(tc, 65533*2, slots_num_adjust(65533*2));
	CuAssertIntEquals(tc, 65533*2+1, slots_num_adjust(65533*2+1));
	CuAssertIntEquals(tc, 65533*2+2, slots_num_adjust(65533*2+2));

	/* across trunk border: note there is a free_header(3slots) for every trunk */
	CuAssertIntEquals(tc, (65533+15*65536-1), slots_num_adjust(65533+15*65536-1));
	CuAssertIntEquals(tc, (65533+15*65536), slots_num_adjust(65533+15*65536));
	CuAssertIntEquals(tc, (65533+15*65536+3), slots_num_adjust(65533+15*65536+1));
	CuAssertIntEquals(tc, (65533+15*65536+3), slots_num_adjust(65533+15*65536+2));
	CuAssertIntEquals(tc, (65533+15*65536+3), slots_num_adjust(65533+15*65536+3));
	CuAssertIntEquals(tc, ((65533+15*65536)*2+3), slots_num_adjust((65533+15*65536)*2+1));
}

void test_init_cvspool(CuTest *tc)
{
	printf("test_init_cvspool() testing...\n");
	int ret;

	cvspool *cp=NULL;
	cvspool_fnode0 *pfn = NULL, *pfn_head = NULL;
	cvspool_trunk *t = NULL;

	/* 1st, init a cvspool with size of 4096Slots*4=16KBytes */
	if ((ret=cvsp_init(&cp, 4096)) != 0)
		printf("cvsp_init() failure...\n");

	CuAssertPtrNotNull(tc, cp);
	t = cp->trunk_list;
	CuAssertPtrNotNull(tc, t);
	CuAssertPtrNotNull(tc, t->mem);
	CuAssertIntEquals(tc, slots_num_adjust(4096), t->t_total);
	CuAssertIntEquals(tc, 0, t->t_used);

	pfn_head = (cvspool_fnode0 *)t->mem;
	pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
	//printf("pfn_head: %lx, pfn: %lx\n", pfn_head, pfn);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, slots_num_adjust(4096), get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(0,0), pfn->idx);

	CuAssertPtrEquals(tc, t->free_head, NEXT_FNODE(t, pfn));
	CuAssertPtrEquals(tc, t->free_head, PREV_FNODE(t, pfn));
	CuAssertPtrEquals(tc, pfn, NEXT_FNODE(t, t->free_head));
	CuAssertPtrEquals(tc, pfn, PREV_FNODE(t, t->free_head));

	for_each_fnode(t, &pfn) {
		printf("line[%d] pfn @ %lx \n", __LINE__, (unsigned long)pfn);
	}

	cvsp_destroy(cp);


	/* 2nd, init a cvspool with size of (2^17+20)Slots*4=(512K+80)Bytes
	 * so, the free list is(node len with slots): 0<-64K-3=64K=20+3->0 */
	if ((ret=cvsp_init(&cp, 128*1024+20)) != 0)
		printf("cvsp_init() failure...\n");

	CuAssertPtrNotNull(tc, cp);
	t = cp->trunk_list;
	CuAssertPtrNotNull(tc, t->mem);
	CuAssertIntEquals(tc, slots_num_adjust(128*1024+20), t->t_total);
	CuAssertIntEquals(tc, 0, t->t_used);

	pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024-3), get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(64*1024), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(64*1024)), pfn->idx);

	cvspool_fnode0 *pp_pfn = pfn;
	pfn = (cvspool_fnode0 *)((char *)t->mem+64*1024*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024), get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(3), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(128*1024)), pfn->idx);

	cvspool_fnode0 *p_pfn = pfn;
	pfn = (cvspool_fnode0 *)((char *)t->mem+128*1024*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	printf("pp_pfn: %lx, p_pfn: %lx, pfn: %lx\n", pp_pfn, p_pfn, pfn);
	CuAssertIntEquals(tc, slots_num_adjust(20+3), get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(64*1024), BLKIDX_OF_SLOTABS(0)), pfn->idx);


	CuAssertPtrEquals(tc, t->free_head, PREV_FNODE(t, pp_pfn));
	CuAssertPtrEquals(tc, pp_pfn, PREV_FNODE(t, p_pfn));
	CuAssertPtrEquals(tc, p_pfn, PREV_FNODE(t, pfn));

	CuAssertPtrEquals(tc, p_pfn, NEXT_FNODE(t, pp_pfn));
	CuAssertPtrEquals(tc, pfn, NEXT_FNODE(t, p_pfn));
	CuAssertPtrEquals(tc, t->free_head, NEXT_FNODE(t, pfn));

	for_each_fnode(t, &pfn) {
		printf("line[%d] pfn @ %lx \n", __LINE__, (unsigned long)pfn);
	}

	cvsp_destroy(cp);

	/* 3rd, init a cvspool with size of (2^16-3)Slots*4=(64K*4-12)Bytes
	 * so, the free list is(node len with slots): 0<-64K-3->0 */
	if ((ret=cvsp_init(&cp, 64*1024-3)) != 0)
		printf("cvsp_init() failure...\n");

	CuAssertPtrNotNull(tc, cp);
	t = cp->trunk_list;
	CuAssertPtrNotNull(tc, t->mem);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024-3), t->t_total);
	CuAssertIntEquals(tc, 0, t->t_used);

	pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024-3), get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(0)), pfn->idx);

	cvsp_destroy(cp);

	/* 4th, init a cvspool with size of (2^16-4)Slots*4=(64K*4-16)Bytes
	 * so, the free list is(node len with slots): 0<-64K-4->0 */
	if ((ret=cvsp_init(&cp, 64*1024-4)) != 0)
		printf("cvsp_init() failure...\n");

	CuAssertPtrNotNull(tc, cp);
	t = cp->trunk_list;
	CuAssertPtrNotNull(tc, t->mem);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024-4), t->t_total);
	CuAssertIntEquals(tc, 0, t->t_used);

	pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024-4), get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(0)), pfn->idx);

	cvsp_destroy(cp);

	/* 5th, init a cvspool with size of (2^16-2)Slots*4=(64K*4-8)Bytes
	 * so, the free list is(node len with slots): 0<-64K-3=3->0 */
	if ((ret=cvsp_init(&cp, 64*1024-2)) != 0)
		printf("cvsp_init() failure...\n");

	CuAssertPtrNotNull(tc, cp);
	t = cp->trunk_list;
	CuAssertPtrNotNull(tc, t->mem);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024-2), t->t_total);
	CuAssertIntEquals(tc, 0, t->t_used);

	pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024-3), get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(64*1024), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(64*1024)), pfn->idx);

	p_pfn = pfn;
	pfn = (cvspool_fnode0 *)((char *)t->mem+64*1024*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 3, get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(3), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(0)), pfn->idx);

	CuAssertPtrEquals(tc, t->free_head, PREV_FNODE(t, p_pfn));
	CuAssertPtrEquals(tc, p_pfn, PREV_FNODE(t, pfn));

	CuAssertPtrEquals(tc, pfn, NEXT_FNODE(t, p_pfn));
	CuAssertPtrEquals(tc, t->free_head, NEXT_FNODE(t, pfn));

	for_each_fnode(t, &pfn) {
		printf("line[%d] pfn @ %lx \n", __LINE__, (unsigned long)pfn);
	}

	cvsp_destroy(cp);

	/* 6th, init a cvspool with size of (2^16)Slots*4=(64K*4)Bytes
	 * so, the free list is(node len with slots): 0<-64K-3=3->0 */
	if ((ret=cvsp_init(&cp, 64*1024-2)) != 0)
		printf("cvsp_init() failure...\n");

	CuAssertPtrNotNull(tc, cp);
	t = cp->trunk_list;
	CuAssertPtrNotNull(tc, t->mem);
	CuAssertIntEquals(tc, slots_num_adjust(64*1024), t->t_total);
	CuAssertIntEquals(tc, 0, t->t_used);

	pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 64*1024-3, get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(64*1024), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(64*1024)), pfn->idx);

	p_pfn = pfn;
	pfn = (cvspool_fnode0 *)((char *)t->mem+64*1024*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 3, get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(3), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(0)), pfn->idx);

	CuAssertPtrEquals(tc, t->free_head, PREV_FNODE(t, p_pfn));
	CuAssertPtrEquals(tc, p_pfn, PREV_FNODE(t, pfn));

	CuAssertPtrEquals(tc, pfn, NEXT_FNODE(t, p_pfn));
	CuAssertPtrEquals(tc, t->free_head, NEXT_FNODE(t, pfn));

	for_each_fnode(t, &pfn) {
		printf("line[%d] pfn @ %lx \n", __LINE__, (unsigned long)pfn);
	}

	cvsp_destroy(cp);

	/* 7th, init a cvspool with size of (2)Slots*4=(2*4)Bytes
	 * so, the free list is(node len with slots): 0<-3->0 */
	if ((ret=cvsp_init(&cp, 2)) != 0)
		printf("cvsp_init() failure...\n");

	CuAssertPtrNotNull(tc, cp);
	t = cp->trunk_list;
	CuAssertPtrNotNull(tc, t->mem);
	CuAssertIntEquals(tc, slots_num_adjust(2), t->t_total);
	CuAssertIntEquals(tc, 0, t->t_used);

	pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 3, get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(0)), pfn->idx);

	cvsp_destroy(cp);

	/* 9th, init a cvspool with size of (1048576*2-4)Slots*4=(1048576*2*4-4*4)Bytes
	 * 1048576*2 Slots is devided to 3 trunks(of size 1048573, 1048573 and 3)
	 * so, the free list are:
	 *	trunk0:	0<-64k-3=64k=64k=64k=...=64k->0, totally 16 Blocks
	 *	trunk1:	0<-64k-3=64k=64k=64k=...=64k->0, totally 16 Blocks
	 *	trunk2:	0<-3->0 */
	if ((ret=cvsp_init(&cp, 2*16*64*1024-4)) != 0)
		printf("cvsp_init() failure...\n");

	CuAssertPtrNotNull(tc, cp);

	/* trunk 0&1 */
	t = cp->trunk_list;

	for (int i=0; i<2; i++) {
		CuAssertPtrNotNull(tc, t->mem);
		CuAssertIntEquals(tc, slots_num_adjust(16*64*1024-3), t->t_total);
		CuAssertIntEquals(tc, 0, t->t_used);

		/* 0th block of trunk 0*/
		pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
		CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
		CuAssertIntEquals(tc, slots_num_adjust(64*1024-3), get_fnode_len(pfn));
		CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(64*1024), pfn->next);
		CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
		CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(64*1024)), pfn->idx);

		/* 1th block of trunk 0*/
		//cvspool_fnode0 *pp_pfn = pfn;
		pfn = (cvspool_fnode0 *)((char *)t->mem+64*1024*4);
		CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
		CuAssertIntEquals(tc, slots_num_adjust(64*1024), get_fnode_len(pfn));
		CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
		CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(3), pfn->prev);
		CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(128*1024)), pfn->idx);

		/* 2th~14th block of trunk 0*/
		for (int j=2; j<=14; j++) {
			pfn = (cvspool_fnode0 *)((char *)t->mem+j*64*1024*4);
			CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
			CuAssertIntEquals(tc, slots_num_adjust(64*1024), get_fnode_len(pfn));
			CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
			CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
			CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS((j-1)*64*1024), BLKIDX_OF_SLOTABS((j+1)*64*1024)), pfn->idx);
		}

		/* 15th block of trunk 0*/
		//cvspool_fnode0 *p_pfn = pfn;
		pfn = (cvspool_fnode0 *)((char *)t->mem+15*64*1024*4);
		CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
		CuAssertIntEquals(tc, slots_num_adjust(64*1024), get_fnode_len(pfn));
		CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
		CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
		CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(14*64*1024), BLKIDX_OF_SLOTABS(0)), pfn->idx);

		t = t->next;
	}

	/* trunk 2 */
	CuAssertPtrNotNull(tc, t->mem);
	CuAssertIntEquals(tc, slots_num_adjust(3), t->t_total);
	CuAssertIntEquals(tc, 0, t->t_used);

	pfn = (cvspool_fnode0 *)(t->mem + (MIN_NODE_SIZE<<2));
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 3, get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	CuAssertIntEquals(tc, PNBLKIDX(BLKIDX_OF_SLOTABS(0), BLKIDX_OF_SLOTABS(0)), pfn->idx);

	/* no other trunks */
	CuAssertPtrNull(tc, t->next);

	cvsp_destroy(cp);
}

void test_alloc1_cvspool(CuTest *tc)
{
	printf("test_alloc1_cvspool() testing...\n");
	cvspool_trunk *t = NULL;

	int ret;
	cvspool *cp=NULL;
	if ((ret=cvsp_init(&cp, 4096)) != 0)
		printf("cvsp_init() failure...\n");

	/* "normal": alloc 12 Bytes */
	char *buf = cvsp_alloc(cp, 12);
	size_t need_slots = 3+1;	// 12Bytes, (3+1) slots totally
	t = cp->trunk_list;
	cvspool_fnode0 *pfn = (cvspool_fnode0 *)(t->mem+(MIN_NODE_SIZE<<2)+need_slots*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 4096-need_slots, get_fnode_len(pfn));	// need_slots=5
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	cvspool_bnode0 *pbn0 = (cvspool_bnode0 *)(buf-4);
	CuAssertIntEquals(tc, BUSY_GUARD0, pbn0->g0);
	CuAssertIntEquals(tc, need_slots, get_bnode_len(pbn0));

	/* "abnormally 1": alloc 10 Bytes */
	buf = cvsp_alloc(cp, 10);
	need_slots = 3+1;	// 10Bytes, (3+1) slots totally
	pfn = (cvspool_fnode0 *)((char *)pfn+need_slots*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 4096-4-need_slots, get_fnode_len(pfn));	// minor 5 for prev 12Bytes
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	pbn0 = (cvspool_bnode0 *)(buf-4);
	CuAssertIntEquals(tc, BUSY_GUARD0, pbn0->g0);
	CuAssertIntEquals(tc, need_slots, get_bnode_len(pbn0));

	/* "abnormally 2": alloc 11 Bytes */
	buf = cvsp_alloc(cp, 11);
	need_slots = 3+1;	// 11Bytes, (3+1) slots totally
	pfn = (cvspool_fnode0 *)((char *)pfn+need_slots*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 4096-4-4-need_slots, get_fnode_len(pfn));	// minor 4 for prev 12Bytes, another 4 for 10Bytes
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	pbn0 = (cvspool_bnode0 *)(buf-4);
	CuAssertIntEquals(tc, BUSY_GUARD0, pbn0->g0);
	CuAssertIntEquals(tc, need_slots, get_bnode_len(pbn0));

	/* "abnormally 3": alloc 13 Bytes */
	buf = cvsp_alloc(cp, 13);
	need_slots = 4+1;	// 13Bytes, (4+1) slots totally
	pfn = (cvspool_fnode0 *)((char *)pfn+need_slots*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 4096-4-4-4-need_slots, get_fnode_len(pfn));
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	pbn0 = (cvspool_bnode0 *)(buf-4);
	CuAssertIntEquals(tc, BUSY_GUARD0, pbn0->g0);
	CuAssertIntEquals(tc, need_slots, get_bnode_len(pbn0));

	/* test the cvsp strcut data */
	CuAssertPtrNotNull(tc, cp);
	CuAssertPtrNotNull(tc, cp->trunk_list->mem);
	CuAssertIntEquals(tc, 4096, cp->total);
	CuAssertIntEquals(tc, (3+1)+(3+1)+(3+1)+(4+1), cp->used);

	/* allocate all remind Bytes */
	buf = cvsp_alloc(cp, 42000);  // have alloated 10+11+12+13 bytes
	CuAssertPtrNull(tc, buf);

	int len_to = cp->total - cp->used;
	buf = cvsp_alloc(cp, (len_to-1)<<2);
	CuAssertPtrEquals(tc, pfn, buf-4);
	CuAssertIntEquals(tc, 4096, cp->used);
	CuAssertIntEquals(tc, len_to, get_bnode_len((cvspool_bnode0 *)(buf-4)));

	for_each_fnode_cp(cp, &t, &pfn) {
		printf("line[%d] pfn @ %lx \n", __LINE__, (unsigned long)pfn);
	}

	cvsp_destroy(cp);
}

void test_alloc2_cvspool(CuTest *tc)
{
	printf("test_alloc2cvspool() testing...\n");

	int ret;
	cvspool *cp=NULL;
	cvspool_fnode0 *pfn = NULL;
	cvspool_trunk *t = NULL;
	int block_cnt=0, total_fslot_cnt=0;

	/* build a tunck size of nearly n*block_size-m */
	if ((ret=cvsp_init(&cp, 1*64*1024-5)) != 0)
		printf("cvsp_init() failure...\n");
	block_cnt=0, total_fslot_cnt=0;
	for_each_fnode_cp(cp, &t, &pfn) {
		//printf("pfn@: %lx, len:%lx\n", (unsigned long)pfn, get_fnode_len(pfn));
		block_cnt++;
		total_fslot_cnt += get_fnode_len(pfn);
	}
	CuAssertIntEquals(tc, 1, block_cnt);	//  4 blocks
	CuAssertIntEquals(tc, 1*64*1024-5, total_fslot_cnt);
	cvsp_destroy(cp);

	/* another n*block_size-m */
	if ((ret=cvsp_init(&cp, 1*64*1024-4)) != 0)
		printf("cvsp_init() failure...\n");

	block_cnt=0, total_fslot_cnt=0;
	for_each_fnode_cp(cp, &t, &pfn) {
		printf("line[%d] pfn@: %lx, len:%lx\n", __LINE__, (unsigned long)pfn, get_fnode_len(pfn));
		block_cnt++;
		total_fslot_cnt += get_fnode_len(pfn);
	}
	CuAssertIntEquals(tc, 1, block_cnt);
	CuAssertIntEquals(tc, 1*64*1024-4, total_fslot_cnt);
	cvsp_destroy(cp);

	/* another n*block_size-3 */
	if ((ret=cvsp_init(&cp, 1*64*1024-3)) != 0)
		printf("cvsp_init() failure...\n");

	block_cnt=0, total_fslot_cnt=0;
	for_each_fnode_cp(cp, &t, &pfn) {
		printf("line[%d] pfn@: %lx, len:%lx\n", __LINE__, (unsigned long)pfn, get_fnode_len(pfn));
		block_cnt++;
		total_fslot_cnt += get_fnode_len(pfn);
	}
	CuAssertIntEquals(tc, 1, block_cnt);	//  4 blocks
	CuAssertIntEquals(tc, 1*64*1024-3, total_fslot_cnt);
	cvsp_destroy(cp);
}


void test_free_cvspool(CuTest *tc)
{
	printf("test_free_cvspool() testing...\n");

	int ret;
	cvspool *cp=NULL;
	if ((ret=cvsp_init(&cp, 4096)) != 0)
		printf("cvsp_init() failure...\n");

	cvspool_trunk *t = cp->trunk_list;

	/* alloc 12, free 12 */
	char *buf = cvsp_alloc(cp, 12);
	size_t need_slots = 3+1;	// 12Bytes, (3+1) slots totally
	cvspool_fnode0 *pfn = (cvspool_fnode0 *)(t->mem+(MIN_NODE_SIZE<<2)+need_slots*4);
	CuAssertIntEquals(tc, FREE_GUARD0, pfn->pad);
	CuAssertIntEquals(tc, 4096-need_slots, get_fnode_len(pfn));	// need_slots=5
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->next);
	CuAssertIntEquals(tc, IBOFF_OF_SLOTABS(0), pfn->prev);
	cvspool_bnode0 *pbn0 = (cvspool_bnode0 *)(buf-4);
	CuAssertIntEquals(tc, BUSY_GUARD0, pbn0->g0);
	CuAssertIntEquals(tc, need_slots, get_bnode_len(pbn0));

	cvsp_free(cp, buf);
	pfn = (cvspool_fnode0 *)(t->mem+(MIN_NODE_SIZE<<2));
	CuAssertIntEquals(tc, 0, cp->used);
	CuAssertPtrEquals(tc, t->free_head, NEXT_FNODE(t, pfn));
	CuAssertPtrEquals(tc, t->free_head, PREV_FNODE(t, pfn));
	CuAssertPtrEquals(tc, pfn, NEXT_FNODE(t, t->free_head));
	CuAssertPtrEquals(tc, pfn, PREV_FNODE(t, t->free_head));

	/* alloc 25+10, free 10 */
	char *buf25 = cvsp_alloc(cp, 25);
	char *buf10 = cvsp_alloc(cp, 10);

	cvsp_free(cp, buf10);
	pfn = (cvspool_fnode0 *)(t->mem+(MIN_NODE_SIZE<<2));
	CuAssertPtrEquals(tc, pfn, buf25-sizeof(cvspool_bnode0));

	pfn = (cvspool_fnode0 *)(buf10-sizeof(cvspool_bnode0));
	CuAssertIntEquals(tc, 8, t->t_used);
	CuAssertPtrEquals(tc, t->free_head, NEXT_FNODE(t, pfn));
	CuAssertPtrEquals(tc, t->free_head, PREV_FNODE(t, pfn));
	CuAssertPtrEquals(tc, pfn, NEXT_FNODE(t, t->free_head));
	CuAssertPtrEquals(tc, pfn, PREV_FNODE(t, t->free_head));


	/* alloc 25+32, free 25,32 */
	char *buf32 = cvsp_alloc(cp, 32);
	cvspool_fnode0 *pfn2 = NEXT_FNODE(t, t->free_head);

	cvsp_free(cp, buf25);
	pfn = (cvspool_fnode0 *)(buf25-sizeof(cvspool_bnode0));
	CuAssertIntEquals(tc, 9, t->t_used);
	CuAssertPtrEquals(tc, t->free_head, NEXT_FNODE(t, pfn));
	CuAssertPtrEquals(tc, pfn2, PREV_FNODE(t, pfn));
	CuAssertPtrEquals(tc, pfn, NEXT_FNODE(t, pfn2));

	cvsp_free(cp, buf32);
	CuAssertIntEquals(tc, 0, cp->used);

	/* alloc 32+41+22, free 41,22,32 */
	buf32 = cvsp_alloc(cp, 32);
	char *buf41 = cvsp_alloc(cp, 41);
	char *buf22 = cvsp_alloc(cp, 22);
	cvsp_free(cp, buf41);
	cvsp_free(cp, buf22);
	cvsp_free(cp, buf32);
	CuAssertIntEquals(tc, 0, cp->used);

	/* alloc 75+48+137+remained, free 48,137,75,remained */
	char *buf75 = cvsp_alloc(cp, 75);
	char *buf48 = cvsp_alloc(cp, 48);
	char *buf137 = cvsp_alloc(cp, 137);
	CuAssertIntEquals(tc, 4096-69, cp->total-cp->used);
	char *buf16102 = cvsp_alloc(cp, 16102);
	CuAssertIntEquals(tc, cp->total, cp->used);
	cvsp_free(cp, buf48);
	cvsp_free(cp, buf137);
	cvsp_free(cp, buf75);
	cvsp_free(cp, buf16102);
	CuAssertIntEquals(tc, 0, cp->used);

	for_each_fnode_cp(cp, &t, &pfn) {
		printf("line[%d] pfn @ %lx \n", __LINE__, (unsigned long)pfn);
	}

	cvsp_destroy(cp);
}

