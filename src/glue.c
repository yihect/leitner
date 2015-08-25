#include "vs.h"

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
void set_offset(unsigned *gn_bitmap, unsigned gn_type, unsigned offset)
{
  unsigned mask=0, total_shift=0;
  
  mask = (1<<bbits(gn_type))-1;
  offset -= LnJ(gn_type);
  offset &= mask;

  while (gn_type > 0) {
    total_shift += bbits(gn_type-1);
    gn_type--;
  }
  offset <<= total_shift;
  mask <<= total_shift;

  *gn_bitmap &= ~mask;
  *gn_bitmap |= offset;
}

unsigned get_offset(unsigned gn_bitmap, unsigned gn_type)
{
  unsigned i=0;

  while (i < gn_type) {
    gn_bitmap >>= bbits(i);
    i++;
  }

  //i as var
  i = gn_bitmap & ((1<<bbits(gn_type))-1);
  return (LnJ(gn_type) + i);
}




