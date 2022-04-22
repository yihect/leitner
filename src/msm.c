#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "msm.h"
#include "log.h"
#include "merror.h"
#include "objpool.h"
#include "cvspool.h"
#include "objvec.h"

static struct ser_root_data *only_srd = NULL;
struct ser_head sh = {"MSM", 0, 1, 0, 0};

extern unsigned int xcrc32 (const unsigned char *buf, int len, unsigned int init);

static size_t ser_osz(struct ser_root_data *srd)
{
	size_t len=0;
	struct ds_node *dsn;

	len += sizeof(struct ser_head);
	len += srd->ds_cnt*sizeof(struct ser_sum_entry);
	list_for_each_entry(dsn, &srd->ds_list, list) {
		len += dsn->get_len(dsn);
	}

	return len;
}

/* serialize data into datav */
void *msm_ser(void *dst_datav, const void *data, size_t sz, bool doser)
{
	if (doser && sz>0)
		memcpy(dst_datav, data, sz);

	return (void*)((char *)dst_datav + sz);
}

/* deserialize data from src_datav */
void *msm_deser(void *src_datav, void *data, size_t sz, bool dodeser)
{
	if (dodeser && sz>0)
		memcpy(data, src_datav, sz);

	return (void*)((char *)src_datav + sz);
}

/* This function expects the caller to have set up a memory buffer of
 * adequate size to hold the serialized content. The sz parameter must be
 * the result of ser_osz().
 */
static int dump_to_mem(struct ser_root_data *srd, void *addr, size_t sz)
{
	void *dv = addr;
	struct ds_node *dsn;
	struct list_head *p, *q;

	/* ser the head */
	struct ser_head *serh = (struct ser_head *)dv;
	dv = msm_ser(dv, &sh, sizeof(struct ser_head), true);

	/* ser every summary entry */
	list_for_each_entry(dsn, &srd->ds_list, list) {
		dv = dsn->enc_summary(dsn, dv);
		serh->sh_cnt++;
	}

	/* ser all details */
	list_for_each_entry(dsn, &srd->ds_list, list) {
		dv = dsn->enc_detail(dsn, dv);
		dsn->sered_sum->sse_off = dsn->sered_detail - addr;
	}

	return 0;
}

static int mmap_output_file(char *filename, size_t sz, void **text_out)
{
	void *text;
	int fd,perms;

	perms = S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP|S_IROTH;  /* ug+w o+r */
	fd=open(filename,O_CREAT|O_TRUNC|O_RDWR,perms);

	if(fd == -1) {
		log_fatal("Couldn't open file %s: %s\n", filename, strerror(errno));
		return -1;
	}

	text = mmap(0, sz, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(text == MAP_FAILED) {
		log_fatal("Failed to mmap %s: %s\n", filename, strerror(errno));
		close(fd);
		return -1;
	}
	if(ftruncate(fd,sz) == -1) {
		log_fatal("ftruncate failed: %s\n", strerror(errno));
		munmap( text, sz );
		close(fd);
		return -1;
	}
	*text_out = text;
	return fd;
}

/* serializing... to mem/file */
int msm_dump(struct ser_root_data *srd, int mode, ...)
{
	va_list ap;
	char *filename, *bufv;
	void **addr_out,*buf, *pa_addr;
	int fd,rc=0;
	size_t sz=0,*sz_out, pa_sz;
	struct stat sbuf;

	if (srd->flags & MSM_RDONLY) {  /* unusual */
		log_fatal("error: dump called for a loaded msm\n");
		return -1;
	}

	/* compute the size needed to serialize  */
	sz = ser_osz(srd);
	assert(sz > 0);

	va_start(ap,mode);
	if (mode & MSM_FILE) {
		filename = va_arg(ap,char*);
		fd = mmap_output_file(filename, sz, &buf);
		if(fd == -1) rc = -1;
		else {
			rc = dump_to_mem(srd,buf,sz);
			if(msync(buf,sz,MS_SYNC) == -1) {
				log_fatal("msync failed on fd %d: %s\n", fd, strerror(errno));
			}
			if(munmap(buf, sz) == -1) {
				log_fatal("munmap failed on fd %d: %s\n", fd, strerror(errno));
			}
			close(fd);
		}
	} else if (mode & MSM_MEM) {
		if (mode & MSM_PREALLOCD) { /* caller allocated */
			pa_addr = (void*)va_arg(ap, void*);
			pa_sz = va_arg(ap, size_t);
			if (pa_sz < sz) {
				log_fatal("msm_dump: buffer too small, need %d bytes\n", sz);
				return -1;
			}
			rc=dump_to_mem(srd,pa_addr,sz);
		} else { /* we allocate */
			addr_out = (void**)va_arg(ap, void*);
			sz_out = va_arg(ap, size_t*);
			if ((buf = malloc(sz)) == NULL)
				log_fatal("out of mem");
			*sz_out = sz;
			*addr_out = buf;
			rc=dump_to_mem(srd,buf,sz);
		}
	} else if (mode & MSM_GETSIZE) {
		sz_out = va_arg(ap, size_t*);
		*sz_out = sz;
	} else {
		log_fatal("unsupported dump mode %d\n", mode);
		rc=-1;
	}
	va_end(ap);
	return rc;
}

static int mmap_file(char *filename, struct ser_root_data *srd)
{
	struct stat stat_buf;

	if ((srd->fd = open(filename, O_RDONLY)) == -1) {
		log_fatal("Couldn't open file %s: %s\n", filename, strerror(errno));
		return -1;
	}

	if (fstat(srd->fd, &stat_buf) == -1) {
		close(srd->fd);
		log_fatal("Couldn't stat file %s: %s\n", filename, strerror(errno));
		return -1;
	}

	srd->text_sz = (size_t)stat_buf.st_size;
	srd->text = mmap(0, stat_buf.st_size, PROT_READ, MAP_PRIVATE, srd->fd, 0);
	if (srd->text == MAP_FAILED) {
		close(srd->fd);
		log_fatal("Failed to mmap %s: %s\n", filename, strerror(errno));
		return -1;
	}

	return 0;
}

struct ds_node *find_dsn_by_name(struct ser_root_data *srd, const char *name);

/* load from memory indicated with addr */
static int load_from_mem(struct ser_root_data *srd, void *addr, size_t sz)
{
	struct ds_node **dsn;
	struct list_head *p, *q;
	struct ser_head lsh;
	void *detail_head, *dv = addr;

	/* deser the head */
	dv = msm_deser(dv, &lsh, sizeof(struct ser_head), true);

	/* calloc will set all byte to zero */
	dsn = calloc(lsh.sh_cnt, sizeof(struct ds_node *));

	/* deser every summary entry */
	for (int i=0; i<lsh.sh_cnt; i++) {
		dsn[i] = find_dsn_by_name(srd, ((struct ser_sum_entry *)dv)->sse_name);
		if (dsn[i] == NULL) log_fatal("find no dsn while deserring...");
		dv = dsn[i]->dec_summary(dsn[i], dv);
	}

	/* deser all details, this is split into two parts:
	 * Part1: deser at higher level, such as loading pools or objects */
	for (int i=0; i<lsh.sh_cnt; i++) {
		assert(dsn[i]->sered_sum->sse_off == (dv-addr));
		detail_head = dv; /* save header */
		if (dsn[i]->sered_sum->sse_len == 0) continue;

		dv = dsn[i]->dec_detail_high(dsn[i], dv);
		assert(dsn[i]->sered_sum->sse_len == (dv-detail_head));
	}

	/* Part2: deser at lower level, namely patching object content internally */
	for (int i=0; i<lsh.sh_cnt; i++) {
		dsn[i]->dec_detail_low(dsn[i]);
	}

	free(dsn);
	return 0;
}

/* loading... from mem/file */
int msm_load(struct ser_root_data *srd, int mode, ...)
{
	va_list ap;
	int rc=0,fd=0;
	char *filename=NULL;
	void *addr;
	size_t sz;

	va_start(ap,mode);
	if (mode & MSM_FILE)
		filename = va_arg(ap,char *);
	else if (mode & MSM_MEM) {
		addr = va_arg(ap,void *);
		sz = va_arg(ap,size_t);
	} else {
		log_fatal("unsupported msm_load mode %d\n", mode);
		return -1;
	}
	va_end(ap);

	if (mode & MSM_FILE) {
		if (mmap_file(filename, srd) != 0) {
			log_fatal("msm_load failed for file %s\n", filename);
			return -1;
		}
		srd->flags = MSM_FILE | MSM_RDONLY;
	} else if (mode & MSM_MEM) {
		srd->text = addr;
		srd->text_sz = sz;
		srd->flags = MSM_MEM | MSM_RDONLY;
	} else {
		log_fatal("invalid msm_load mode %d\n", mode);
		return -1;
	}

	/* load serialized content */
	load_from_mem(srd, srd->text, srd->text_sz);

	return 0;
}

void msm_register_ds(struct ser_root_data *srd, struct ds_node *ds, enum ptype p_type, void *p_pt)
{
	assert(ds != NULL);
	if (ds->get_len == NULL)	ds->get_len = def_getlen;
	if (ds->enc_summary == NULL)	ds->enc_summary = def_enc_summary;
	if (ds->enc_detail == NULL)	ds->enc_detail = def_enc_detail;
	if (ds->dec_summary == NULL)	ds->dec_summary = def_dec_summary;
	if (ds->dec_detail_high == NULL)ds->dec_detail_high = def_dec_detail_high;
	if (ds->dec_detail_low == NULL)	ds->dec_detail_low = def_dec_detail_low;

	ds->pool_pt = p_pt;
	ds->pool_type = p_type;
	ds->srd = srd;
	list_add_tail(&ds->list, &srd->ds_list);
	srd->ds_cnt++;
}

void msm_change_ds_pt(struct ser_root_data *srd, struct ds_node *ds, void *new_p_pt)
{
	assert(find_dsn_by_name(srd, ds->name) != NULL);
	ds->pool_pt = new_p_pt;
}

struct ds_node *find_dsn_by_name(struct ser_root_data *srd, const char *name)
{
	struct ds_node *dsn;
	assert(name != NULL);

	list_for_each_entry(dsn, &srd->ds_list, list) {
		if (!strcmp(dsn->name, name))
		    break;
	}

	return dsn;
}

void msm_register_pt(struct ser_root_data *srd, enum ptype p_type,
		    unsigned int (*getlen)(struct ser_root_data *srd, void *mpl),
		    char *(*ser)(struct ser_root_data *srd, void *mpl, char *dst),
		    char *(*deser_high)(struct ser_root_data *srd, void *mpl, char *src),
		    char *(*deser_low)(struct ser_root_data *srd, void *mpl))
{
	srd->pvec[p_type].getlen = getlen;
	srd->pvec[p_type].ser = ser;
	srd->pvec[p_type].deser_high = deser_high;
	srd->pvec[p_type].deser_low = deser_low;
}

/* NOTE: this function must be called in *_dec_detail_low() etc,
 * because that there should be old_id->newid mapping in ididr */
void *get_new_pool_pt(struct ser_root_data *srd, int old_id)
{
	/* get new pool id and then new pool ptr */
	int newid = idr_find(&srd->ididr, (int)old_id);

	assert(newid != 0);
	return idr_find(&srd->poolidr, newid);
}

struct ser_root_data *msm_get()
{
	struct ser_root_data * srd = msm_get_no_ref();
	srd->refcnt++;

	return srd;
}

struct ser_root_data *msm_get_no_ref()
{
	if (only_srd != NULL)
		return only_srd;
	only_srd = malloc(sizeof(struct ser_root_data));

	memset(only_srd, 0, sizeof(struct ser_root_data));
	INIT_LIST_HEAD(&only_srd->ds_list);
	idr_init(&only_srd->poolidr);
	idr_init(&only_srd->ididr);

	/* init pool ops */
	msm_register_pt(only_srd, PT_OBJP, objpool_ser_getlen,
			objpool_ser, objpool_deser_high, objpool_deser_low);
	msm_register_pt(only_srd, PT_CVSP, cvspool_ser_getlen,
			cvspool_ser, cvspool_deser_high, cvspool_deser_low);
	msm_register_pt(only_srd, PT_OBJV, objvec_ser_getlen,
			objvec_ser, objvec_deser_high, objvec_deser_low);
	msm_register_pt(only_srd, PT_NOP, NULL, NULL, NULL, NULL);

	return only_srd;
}

void msm_put(struct ser_root_data *srd)
{
	assert(srd == only_srd);

	if (only_srd->refcnt-- == 0) {
		free(only_srd);
		only_srd = NULL;
	}
}


/* default functions for serializing */
unsigned int def_getlen(struct ds_node *dn)
{
	struct ser_root_data *srd = dn->srd;

	assert(srd != NULL);
	assert((dn->pool_type>=PT_OBJP) || (dn->pool_type<PT_MAX));

	return srd->pvec[dn->pool_type].getlen(srd, dn->pool_pt);
}

void *def_enc_summary(struct ds_node *dsn, char *dst)
{
	struct ser_sum_entry sse = {0};

	dsn->sered_sum = dst;
	strncpy(sse.sse_name, dsn->name, sizeof(sse.sse_name));
	return msm_ser(dst, &sse, sizeof(struct ser_sum_entry), true);
}

void *def_enc_detail(struct ds_node *dn, char *dst)
{
	struct ser_root_data *srd = dn->srd;
	struct ser_sum_entry *sse = dn->sered_sum;

	assert(srd != NULL);
	assert((dn->pool_type>=PT_OBJP) || (dn->pool_type<PT_MAX));

	dn->sered_detail = dst;
	void *dend = srd->pvec[dn->pool_type].ser(srd, dn->pool_pt, dst);
	sse->sse_len = (char *)dend - dst;

	return dend;
}

void *def_dec_summary(struct ds_node *dn, char *src)
{
	dn->sered_sum = src;

	return src + sizeof(struct ser_sum_entry);
}

void *def_dec_detail_high(struct ds_node *dn, char *src)
{
	struct ser_root_data *srd = dn->srd;

	assert(srd != NULL);
	assert((dn->pool_type>=PT_OBJP) || (dn->pool_type<PT_MAX));

	return srd->pvec[dn->pool_type].deser_high(srd, dn->pool_pt, src);
}

void def_dec_detail_low(struct ds_node *dn)
{
	struct ser_root_data *srd = dn->srd;

	assert(srd != NULL);
	srd->pvec[dn->pool_type].deser_low(srd, dn->pool_pt);
}

