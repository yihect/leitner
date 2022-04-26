#include <assert.h>
#include "glue.h"

DEFINE_ENUM(business_data_types, BUSINESS_DATA_TYPES, strlen("BDT_"))

DEFINE_ENUM(phontrans_ts_types, PHONTRANS_TS_TYPES, strlen("TS_PT_"))
DEFINE_ENUM(phonograms_ts_types, PHONOGRAMS_TS_TYPES, strlen("TS_PG_"))
DEFINE_ENUM(rootfixes_ts_types, ROOTFIXES_TS_TYPES, strlen("TS_RF_"))
DEFINE_ENUM(vocabulary_ts_types, VOCABULARY_TS_TYPES, strlen("TS_V_"))
DEFINE_ENUM(grammar_ts_types, GRAMMAR_TS_TYPES, strlen("TS_G_"))
DEFINE_ENUM(sentence_ts_types, SENTENCE_TS_TYPES, strlen("TS_S_"))

/* all processing of UW_XX_ ids ignores 6 heading chars */
DEFINE_ENUM(uw_types, UW_TYPES, strlen("UW_XX_"))

DEFINE_ENUM(glue_lh_node_types, GLUE_LH_NODE_TYPES, strlen("GLT_"))

struct glue_info gi_eng[GLT_GLTMAX] =
{
  [GLT_UW_RFIX] = {
	  .gie_tbl = {
		  [GLOF_MW_RFIX_TYPE_LHEAD] = { /* -q-> */
			  GLOF_MW_RFIX_TYPE_LHEAD, GLOF_MW_RFIX_TYPE_LNODE,
			  GLIT_LHEAD_1TON,
			  "All Roots or Fixes of this type:"
		  },
	  },
  },
  [GLT_MW_RFIX] = {
	  .gie_tbl = {
		  [GLOF_MW_RFIX_TYPE_LNODE] = { /* <-q- */
			  GLOF_MW_RFIX_TYPE_LNODE, GLOF_MW_RFIX_TYPE_LHEAD,
			  GLIT_LNODE_TOP_BACKPT,
			  "The type of this Root or Fix:"
		  },
		  [GLOF_TW_RFIX_USAGE_LHEAD] = { /* -r-> */
			  GLOF_TW_RFIX_USAGE_LHEAD, GLOF_TW_RFIX_USAGE_LNODE,
			  GLIT_LHEAD_1TON,
			  "All usage rules of this Root or Fix:"
		  },
		  [GLOF_MW_RFIX_INSTANCE_LHEAD] = { /* -s-> */
			  GLOF_MW_RFIX_INSTANCE_LHEAD, GLOF_MW_VOCA_RTINCLUDED_LHEAD,
			  GLIT_LHEAD_MTON,
			  "All usage instances of this Root or Fix:"
		  },
		  [GLOF_MW_VOCA_RTINCLUDED_LNODE] = { /* <-v- */
			  GLOF_MW_VOCA_RTINCLUDED_LNODE, GLOF_MW_VOCA_RTINCLUDED_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
	  },
  },
  [GLT_TW_RFIX] = {
	  .gie_tbl = {
		  [GLOF_TW_RFIX_USAGE_LNODE] = { /* <-r- */
			  GLOF_TW_RFIX_USAGE_LNODE, GLOF_TW_RFIX_USAGE_LHEAD,
			  GLIT_LNODE_TOP_BACKPT,
			  "The Root or Fix of this usage rule:"
		  },
		  [GLOF_MW_RFIX_USAGE_INSTANCE_LHEAD] = { /* -t-> */
			  GLOF_MW_RFIX_USAGE_INSTANCE_LHEAD, GLOF_MW_RFIX_USAGE_INSTANCE_LNODE,
			  GLIT_LHEAD_MTON,
			  "All vocab examples of this Roots/Fixes usage instance:"
		  },
		  [GLOF_TW_VOCA_USAGE_RTINCLUDED_LNODE] = { /* <-u- */
			  GLOF_TW_VOCA_USAGE_RTINCLUDED_LNODE, GLOF_TW_VOCA_USAGE_RTINCLUDED_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
	  },
  },
  [GLT_UW_VOCA] = {
	  .gie_tbl = {
		  [GLOF_MW_VOCA_CATEWORDS_LHEAD] = { /* -i-> */
			  GLOF_MW_VOCA_CATEWORDS_LHEAD, GLOF_UW_VOCA_CATEWORDS_LNODE,
			  GLIT_LHEAD_1TON,
			  "All vocas of this category:"
		  },
		  [GLOF_MW_VOCA_CATE_LNODE] = { /* <-k- */
			  GLOF_MW_VOCA_CATE_LNODE, GLOF_UW_VOCA_CATE_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
	  },
  },
  [GLT_MW_VOCA] = { /* vocab mw */
	  .gie_tbl = {
		  [GLOF_MW_VOCA_BASEWORDS_LNODE] = { /* <-j- */
			  GLOF_MW_VOCA_BASEWORDS_LNODE, GLOF_MW_VOCA_BASEWORDS_LHEAD,
			  GLIT_LNODE_TOP_BACKPT,
			  "The base word of this vocabulay:"
		  },
		  [GLOF_UW_VOCA_CATE_LHEAD] = { /* -k-> */
			  GLOF_UW_VOCA_CATE_LHEAD, GLOF_MW_VOCA_CATE_LNODE,
			  GLIT_LHEAD_1TON,
			  "All categories of this vocabulary:"
		  },
		  [GLOF_SW_VOCA_EXAMPLEV_LHEAD] = { /* -l-> */
			  GLOF_SW_VOCA_EXAMPLEV_LHEAD, GLOF_MW_VOCA_EXAMPLEV_LNODE,
			  GLIT_LHEAD_1TON,
			  "All example sentences of this vocabulay:"
		  },
		  [GLOF_MW_VOCA_BASEWORDS_LHEAD] = { /* -j-> */
			  GLOF_MW_VOCA_BASEWORDS_LHEAD, GLOF_MW_VOCA_BASEWORDS_LNODE,
			  GLIT_LHEAD_1TON,
			  "All derivative words of this base word:"
		  },
		  [GLOF_TW_VOCA_MEANINGUSAGE_LHEAD] = { /* -n-> */
			  GLOF_TW_VOCA_MEANINGUSAGE_LHEAD, GLOF_TW_VOCA_MEANINGUSAGE_LNODE,
			  GLIT_LHEAD_1TON,
			  "All meaning or usage info of this vocabulary:"
		  },
		  [GLOF_MW_RFIX_INSTANCE_LNODE] = { /* <-s- */
			  GLOF_MW_RFIX_INSTANCE_LNODE, GLOF_MW_RFIX_INSTANCE_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
		  [GLOF_MW_VOCA_RTINCLUDED_LHEAD] = { /* -v-> */
			  GLOF_MW_VOCA_RTINCLUDED_LHEAD, GLOF_MW_VOCA_RTINCLUDED_LNODE,
			  GLIT_LHEAD_1TON,
			  "All roots/fixes included in this vocabulary:"
		  },
		  [GLOF_TW_VOCA_WORDSALLG_LHEAD] = { /* -d-> */
			  GLOF_TW_VOCA_WORDSALLG_LHEAD, GLOF_TW_VOCA_WORDSALLG_LNODE,
			  GLIT_LHEAD_1TON,
			  "All grammar points of this vocabulary:"
		  },
		  [GLOF_UW_VOCA_CATEWORDS_LNODE] = {	/* <-i- */
			  GLOF_UW_VOCA_CATEWORDS_LNODE, GLOF_MW_VOCA_CATEWORDS_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
	  },
  },
  [GLT_TW_VOCA] = { /* vocab tw */
	  .gie_tbl = {
		  [GLOF_TW_VOCA_MEANINGUSAGE_LNODE] = {	/* <-n- */
			  GLOF_TW_VOCA_MEANINGUSAGE_LNODE, GLOF_TW_VOCA_MEANINGUSAGE_LHEAD,
			  GLIT_LNODE_TOP_BACKPT,
			  "The vocabulary of this meaning or usage:"
		  },
#if 0
		  [GLOF_TW_VOCA_TYPEUSAGE_LNODE] = {
			  GLOF_TW_VOCA_TYPEUSAGE_LNODE, GLOF_TW_VOCA_TYPEUSAGE_LHEAD,
			  GLIT_LNODE_BOTTOM_BACKPT,
			  "The vocab type of this meaning or usage used as:"
		  },
#endif
		  [GLOF_TW_VOCA_DUMMY1] = {
			  GLOF_TW_VOCA_DUMMY1, GLOF_TW_VOCA_DUMMY1,
			  GLIT_DUMMY,
			  "Dummy:"
		  },
		  [GLOF_SW_VOCA_EXAMPLEVUSAGE_LHEAD] = {
			  GLOF_SW_VOCA_EXAMPLEVUSAGE_LHEAD, GLOF_TW_SENT_VUSEDUSAGE_LHEAD,
			  GLIT_LHEAD_MTON,
			  "All sent examples of this usage instance of this vocab:"
		  },
		  [GLOF_MW_RFIX_USAGE_INSTANCE_LNODE] = { /* <-t- */
			  GLOF_MW_RFIX_USAGE_INSTANCE_LNODE, GLOF_MW_RFIX_USAGE_INSTANCE_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
		  [GLOF_TW_VOCA_USAGE_RTINCLUDED_LHEAD] = { /* -u-> */
			  GLOF_TW_VOCA_USAGE_RTINCLUDED_LHEAD, GLOF_TW_VOCA_USAGE_RTINCLUDED_LNODE,
			  GLIT_LHEAD_1TON,
			  "All Roots/Fixes usage rules in this vocabulary:"
		  },
	  },
  },
  [GLT_UW_GRAM] = { /* grammar tw */
	  .gie_tbl = {
		  [GLOF_MW_GRAM_GTYPE_LHEAD] = { /* -a-> */
			  GLOF_MW_GRAM_GTYPE_LHEAD, GLOF_UW_GRAM_GTYPE_LNODE,
			  GLIT_LHEAD_1TON,
			  "All grammar sub-classes of this big type:"
		  },
	  },
  },
  [GLT_MW_GRAM] = { /* grammar tw */
	  .gie_tbl = {
		  [GLOF_UW_GRAM_GTYPE_LNODE] = { /* <-a- */
			  GLOF_UW_GRAM_GTYPE_LNODE, GLOF_MW_GRAM_GTYPE_LHEAD,
			  GLIT_LNODE_TOP_BACKPT,
			  "The grammar big type of this sub-class:"
		  },
		  [GLOF_TW_GRAM_GUSAGE_LHEAD] = { /* -b-> */
			  GLOF_TW_GRAM_GUSAGE_LHEAD, GLOF_MW_GRAM_GUSAGE_LNODE,
			  GLIT_LHEAD_1TON,
			  "All small grammar usage points in this sub-class:"
		  },
		  [GLOF_MW_GRAM_EXAMPLEG_LHEAD] = { /* -e-> */
			  GLOF_MW_GRAM_EXAMPLEG_LHEAD, GLOF_MW_GRAM_EXAMPLEG_LNODE,
			  GLIT_LHEAD_1TON,
			  "All sentence examples for this grammar sub-class:"
		  },
		  [GLOF_MW_SENT_GUSED_LNODE] = { /* <-g- */
			  GLOF_MW_SENT_GUSED_LNODE, GLOF_MW_SENT_GUSED_LHEAD,
			  GLIT_LNODE,
			  "NULL",
		  },
	  },
  },
  [GLT_TW_GRAM] = { /* grammar tw */
	  .gie_tbl = {
		  [GLOF_MW_GRAM_GUSAGE_LNODE] = { /* <-b- */
			  GLOF_MW_GRAM_GUSAGE_LNODE, GLOF_TW_GRAM_GUSAGE_LHEAD,
			  GLIT_LNODE_TOP_BACKPT,
			  "The grammar sub-class of this small usage point:"
		  },
		  [GLOF_TW_GRAM_VWORDS_LHEAD] = { /* -c-> */
			  GLOF_TW_GRAM_VWORDS_LHEAD, GLOF_TW_VOCA_WORDSALLG_LHEAD,
			  GLIT_LHEAD_MTON,
			  "All vocabularies this grammar point controls:"
		  },
		  [GLOF_MW_GRAM_EXAMPLEUSAGE_LHEAD] = { /* -f-> */
			  GLOF_MW_GRAM_EXAMPLEUSAGE_LHEAD, GLOF_MW_GRAM_EXAMPLEUSAGE_LNODE,
			  GLIT_LHEAD_1TON,
			  "All sent examples of this grammar point:"
		  },
		  [GLOF_TW_VOCA_WORDSALLG_LNODE] = { /* <-d- */
			  GLOF_TW_VOCA_WORDSALLG_LNODE, GLOF_TW_VOCA_WORDSALLG_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
		  [GLOF_TW_SENT_GUSEDUSAGE_LNODE] = { /* <-h- */
			  GLOF_TW_SENT_GUSEDUSAGE_LNODE, GLOF_TW_SENT_GUSEDUSAGE_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
	  },
  },
  [GLT_MW_SENTENCE] = { /* sentence tw */
	  .gie_tbl = {
		  [GLOF_MW_SENT_VUSED_LHEAD] = {
			  GLOF_MW_SENT_VUSED_LHEAD, GLOF_SW_VOCA_EXAMPLEV_LHEAD,
			  GLIT_LHEAD_MTON,
			  "All key vocabularies used in this sentence:"
		  },
		  [GLOF_TW_SENT_VUSEDUSAGE_LHEAD] = {
			  GLOF_TW_SENT_VUSEDUSAGE_LHEAD, GLOF_SW_VOCA_EXAMPLEVUSAGE_LHEAD,
			  GLIT_LHEAD_MTON,
			  "All vocab detail usage info in this sentence:"
		  },
		  [GLOF_MW_VOCA_EXAMPLEV_LNODE] = { /* <-l- */
			  GLOF_MW_VOCA_EXAMPLEV_LNODE, GLOF_SW_VOCA_EXAMPLEV_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
		  [GLOF_MW_SENT_GUSED_LHEAD] = { /* -g-> */
			  GLOF_MW_SENT_GUSED_LHEAD, GLOF_MW_SENT_GUSED_LNODE,
			  GLIT_LHEAD_MTON,
			  "All grammar sub-classes used in this sentence:"
		  },
		  [GLOF_TW_SENT_GUSEDUSAGE_LHEAD] = { /* -h-> */
			  GLOF_TW_SENT_GUSEDUSAGE_LHEAD, GLOF_TW_SENT_GUSEDUSAGE_LNODE,
			  GLIT_LHEAD_1TON,
			  "All grammar points used in this sentence:"
		  },
		  [GLOF_MW_GRAM_EXAMPLEG_LNODE] = { /* <-e- */
			  GLOF_MW_GRAM_EXAMPLEG_LNODE, GLOF_MW_GRAM_EXAMPLEG_LHEAD,
			  GLIT_LNODE,
			  NULL,
		  },
		  [GLOF_MW_GRAM_EXAMPLEUSAGE_LNODE] = {	/* <-f- */
			  GLOF_MW_GRAM_EXAMPLEUSAGE_LNODE, GLOF_MW_GRAM_EXAMPLEUSAGE_LHEAD,
			  GLIT_LHEAD_1TON,
			  "All sent examples of this grammar point:"
		  },
	  },
  },
};

