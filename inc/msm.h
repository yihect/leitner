#ifndef _MSM_H_
#define _MSM_H_

#include <stdlib.h>
#include <stdbool.h>
#include "list.h"
#include "idr.h"

/* MSM is a Multi-sourced data Serialization Module */

/* datasource/pool ID range */
#define POOLID_MIN	1
#define POOLID_MAX	512	/* could be 2^10=1024?? */

/* bit flags (external) */
#define MSM_FILE	(1 << 0)
#define MSM_MEM		(1 << 1)
#define MSM_PREALLOCD	(1 << 2)
#define MSM_FD		(1 << 3)
#define MSM_GETSIZE	(1 << 4)
#define MSM_RDONLY	(1 << 5)  /* was loaded (for unpacking) */

struct pool_ops {
	unsigned int (*getlen)(struct ser_root_data *srd, void *mpl);
	char *(*ser)(struct ser_root_data *srd, void *mpl, char *dst);
	char *(*deser_high)(struct ser_root_data *srd, void *mpl, char *src);
	char *(*deser_low)(struct ser_root_data *srd, void *mpl);
};

/* pool type, transed into msm_register_ds() */
enum ptype {
	PT_OBJP,
	PT_CVSP,
	PT_OBJV,
	PT_NOP,
	PT_MAX,
};

struct ser_root_data {
	int ds_cnt;	/* data source cnt */
	struct list_head ds_list;	/* data source list */
	struct pool_ops pvec[PT_MAX];
	int flags;
	int refcnt;

	int fd;
	void *text;
	size_t text_sz;

	/* for pool ptr stored in other struct: deser old pool id to new ptr */
	struct idr poolidr;	/* pool id -> pool ptr */
	struct idr ididr;	/* when desering, old pool id -> new pool id */
};

/* serialization header */
struct ser_head {
	char sh_magic[8];
	unsigned int sh_cnt;	/* cnt of summary entry */
	unsigned int sh_ver;	/* version */
#if 0
	unsigned int sh_hcrc;	/* crc for ser_head */
	unsigned int sh_dcrc;	/* crc for data */
#endif
};

/* entries saved in summary table */
struct ser_sum_entry {
	char sse_name[8];	/* saved ds_node.name */
	unsigned int sse_off;	/* offset of detail in serialized file|mem */
	unsigned int sse_len;	/* saved len of detail */
};

/* data source node */
struct ds_node {
	const char *name;
	struct list_head list;
	struct ser_root_data *srd;

	void *pool_pt;			/* private of upper layer, usually pointer of pool */
	unsigned char pool_type;	/* cvsp or objp or objv */

	struct ser_sum_entry *sered_sum;	/* sum entry ptr */
	void *sered_detail;		/* detail ptr */

	unsigned int (*get_len)(struct ds_node *dn);	/* detail len, in bytes */
	void *(*enc_summary)(struct ds_node *dn, void *dst);
	void *(*enc_detail)(struct ds_node *dn, void *dst);
	void *(*dec_summary)(struct ds_node *dn, void *src);
	void *(*dec_detail_high)(struct ds_node *dn, void *src);
	void (*dec_detail_low)(struct ds_node *dn);
};

struct ser_root_data *msm_get();
struct ser_root_data *msm_get_no_ref();
void msm_put(struct ser_root_data *srd);

void *get_new_pool_pt(struct ser_root_data *srd, int old_id);

void msm_register_ds(struct ser_root_data *srd, struct ds_node *ds, enum ptype p_type, void *p_pt);
void msm_register_pt(struct ser_root_data *srd, enum ptype p_type,
		    unsigned int (*getlen)(struct ser_root_data *srd, void *mpl),
		    char *(*ser)(struct ser_root_data *srd, void *mpl, char *dst),
		    char *(*deser_high)(struct ser_root_data *srd, void *mpl, char *src),
		    char *(*deser_low)(struct ser_root_data *srd, void *mpl));
void msm_change_ds_pt(struct ser_root_data *srd, struct ds_node *ds, void *new_p_pt);

/* serialize or deserialize data into or from des|src_datav */
void *msm_ser(void *dst_datav, const void *data, size_t sz, bool doser);
void *msm_deser(void *src_datav, void *data, size_t sz, bool dodeser);

int msm_dump(struct ser_root_data *srd, int mode, ...);
int msm_load(struct ser_root_data *srd, int mode, ...);

/* default functions for serializing */
unsigned int def_getlen(struct ds_node *dn);
void *def_enc_summary(struct ds_node *dn, char *dst);
void *def_enc_detail(struct ds_node *dn, char *dst);
void *def_dec_summary(struct ds_node *dn, char *src);
void *def_dec_detail_high(struct ds_node *dn, char *src);
void def_dec_detail_low(struct ds_node *dn);

#endif /* _MSM_H_ */


