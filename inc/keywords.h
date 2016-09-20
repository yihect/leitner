
#ifndef KEYWORD
/* do_xxx(); prototype for keyword xxx */

#define __MAKE_KEYWORD_ENUM__
#define KEYWORD(symbol, symlen, flags, nargs, func, line_func) K_##symbol,
enum {
    K_UNKNOWN,
#endif

    /* Note, we must maintain lookup_keyword() accordingly */
    KEYWORD(ENUM, 4, SECTION, 0, 0, 0) 
    KEYWORD(RFX, 3, SECTION, 0, parse_rfx, parse_line_rfx) 
    KEYWORD(VOC, 3, SECTION, 0, parse_voc, parse_line_voc) 
    KEYWORD(VGRP, 4, SECTION, 0, parse_vgrp, parse_line_vgrp) 
    KEYWORD(GRAM, 4, SECTION, 0, parse_gram, parse_line_gram) 
    KEYWORD(VS, 2, SECTION, 0, parse_vs, parse_line_vs) 
    KEYWORD(ITEM, 4, SECTION, 0, parse_item, parse_line_item) 
    KEYWORD(RFREF, 5, OPTION, 0, 0, 0) 
    KEYWORD(VREF, 4, OPTION, 0, 0, 0) 
    KEYWORD(VGREF, 5, OPTION, 0, 0, 0) 
    KEYWORD(GREF, 4, OPTION, 0, 0, 0) 

#ifdef __MAKE_KEYWORD_ENUM__
    KEYWORD_COUNT,
};
#undef __MAKE_KEYWORD_ENUM__
#undef KEYWORD
#endif

