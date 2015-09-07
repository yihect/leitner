#include "vs.h"

/*
 * Glue bitmap format in gl node:
 *
 * a, type 0, length of 4 Bytes
 * 
 *  Note: T=0, P is free
 *
 * 31  28 27 26         18 17  15 14	             0
 *  XXXX  T   YYY YYYY YY   PP P   FFF FFFF FFFF FFFF
 *  ----  ------- ---- ------- ------- ---- ---- ----
 *  TYPE      <ValidBits>          <--Offset   val-->
 *
 * Totally encode 9 lh nodes in Offset field 
 * with 9bits in ValidBits field.
 *
 *
 * b, type 1, length of 8 Bytes
 *
 *  Note: T=1, P is free
 *
 * 31  28 27 26                   11 9            0
 *  XXXX  T   YYY YYYY YYYY YYYY YY   FF FFFF FFFF
 *  ----  ------- ---- ---- ---- ------- ---- ----
 *  TYPE      <---Valid----Bits--->   <--Offset-->
 *
 * 3130 29                                    0
 *  PP   FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
 *  ------- ---- ---- ---- ---- ---- ---- ----
 *       <-----Offset----------------val----->
 *
 * The 1st Byte encode 3 lh nodes, and the 2nd
 * Byte encode 14 ln nodes. 
 *
 *
 */

#include "./glue_info.c"

///////////////////////
//  help functions

/* for LnJ */
inline unsigned LnJ(unsigned n)
{
  unsigned r=1;
  
  if (n<2) return 0;
  while(r<=n) r<<=1;	

  return r>>1;
}


/* binary bits of unsigned n:
 * = log2LnJ */
inline unsigned bbits(unsigned n)
{
  unsigned r=0;
  
  if (n<2) return 1;
  n = LnJ(n); // for LnJ
  while ((n>>=1)>0) r++;  // for log2LnJ
  return r;
}

///////////////////////
// logical funcs

unsigned is_offset_valid(unsigned *gn_bitmap, unsigned gn_type)
{
  unsigned vshift_bits = ((*gn_bitmap & GBM_FORMAT_TYPE_MASK) ? GBM_FT1_VALIDBITS_SHIFT
    							: GBM_FT0_VALIDBITS_SHIFT);
  if (*gn_bitmap & GBM_FORMAT_TYPE_MASK)
    BUG_ON(gn_type >= GBM_FT1_VALIDBITS_SIZE);
  else
    BUG_ON(gn_type >= GBM_FT0_VALIDBITS_SIZE);

  return (0 != (*gn_bitmap & (1<<(gn_type+vshift_bits))));
}

void mark_offset(unsigned *gn_bitmap, unsigned gn_type, unsigned validness)
{
  unsigned vshift_bits = ((*gn_bitmap & GBM_FORMAT_TYPE_MASK) ? GBM_FT1_VALIDBITS_SHIFT
    							: GBM_FT0_VALIDBITS_SHIFT);
  if (*gn_bitmap & GBM_FORMAT_TYPE_MASK)
    BUG_ON(gn_type >= GBM_FT1_VALIDBITS_SIZE);
  else
    BUG_ON(gn_type >= GBM_FT0_VALIDBITS_SIZE);

  if (validness == true)
    *gn_bitmap |= 1<<(gn_type+vshift_bits);
  else
    *gn_bitmap &= ~(1<<(gn_type+vshift_bits));
}

static inline void _set_offset(
    unsigned *gn_bitmap, unsigned gn_type, unsigned start_type, unsigned offset) 
  __attribute__((always_inline));
void _set_offset(unsigned *gn_bitmap, unsigned gn_type, unsigned start_type, unsigned offset)
{
  unsigned mask=0, total_shift=0;
  
  mask = (1<<bbits(gn_type))-1;
  offset -= LnJ(gn_type);
  offset &= mask;

  BUG_ON(gn_type < start_type);
  while (gn_type > start_type) {
    total_shift += bbits(gn_type-1);
    gn_type--;
  }
  offset <<= total_shift;
  mask <<= total_shift;

  *gn_bitmap &= ~mask;
  *gn_bitmap |= offset;
}

void set_offset(unsigned *gn_bitmap, unsigned gn_type, unsigned offset)
{
  BUG_ON(offset<LnJ(gn_type) || offset>2*LnJ(gn_type));
  if (*gn_bitmap & GBM_FORMAT_TYPE_MASK)
  {
    BUG_ON(gn_type >= GBM_FT1_VALIDBITS_SIZE);
    if (gn_type < GBM_FT1_2ND_LH_ITEMS)
      _set_offset(gn_bitmap+1, gn_type, 0, offset);
    else
      _set_offset(gn_bitmap, gn_type, GBM_FT1_2ND_LH_ITEMS, offset);
    *gn_bitmap |= 1<<(gn_type+GBM_FT1_VALIDBITS_SHIFT);
  }
  else
  {
    BUG_ON(gn_type >= GBM_FT0_VALIDBITS_SIZE);
    _set_offset(gn_bitmap, gn_type, 0, offset);
    *gn_bitmap |= 1<<(gn_type+GBM_FT0_VALIDBITS_SHIFT);
  }
}

static inline unsigned _get_offset(
    unsigned gn_bitmap, unsigned gn_type, unsigned start_type)
  __attribute__((always_inline));
unsigned _get_offset(unsigned gn_bitmap, unsigned gn_type, unsigned start_type)
{
  unsigned i=start_type;

  BUG_ON(gn_type < start_type);
  while (i < gn_type) {
    gn_bitmap >>= bbits(i);
    i++;
  }

  //i as var
  i = gn_bitmap & ((1<<bbits(gn_type))-1);
  return (LnJ(gn_type) + i);
}

unsigned get_offset(unsigned *gn_bitmap, unsigned gn_type)
{
  unsigned res_off=0;

  if (*gn_bitmap & GBM_FORMAT_TYPE_MASK)
  {
    BUG_ON(gn_type >= GBM_FT1_VALIDBITS_SIZE);
    if (gn_type < GBM_FT1_2ND_LH_ITEMS)
      res_off = _get_offset(*(gn_bitmap+1), gn_type, 0);
    else
      res_off = _get_offset(*gn_bitmap, gn_type, GBM_FT1_2ND_LH_ITEMS);
  }
  else
  {
    BUG_ON(gn_type >= GBM_FT0_VALIDBITS_SIZE);
    res_off = _get_offset(*gn_bitmap, gn_type, 0);
  }

  return res_off;
}






