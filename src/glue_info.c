
struct glue_info gi[GLT_MAX] =
{
  [GLT_UW_RFIX] = { 
	  .gie_tbl = { 
		  [GLOF_MW_RFIX_TYPE_LHEAD] = {
			  GLOF_MW_RFIX_TYPE_LHEAD, GLOF_MW_RFIX_TYPE_LNODE, 
			  GLT_LHEAD_1TON, 
			  "All Roots or Fixes of this type:"
		  },
	  },
  },
  [GLT_MW_RFIX] = { 
	  .gie_tbl = { 
		  [GLOF_MW_RFIX_TYPE_LNODE] = {
			  GLOF_MW_RFIX_TYPE_LNODE, GLOF_MW_RFIX_TYPE_LHEAD, 
			  GLT_LNODE_TOP_BACKPT, 
			  "The type of this Root or Fix:"
		  },
		  [GLOF_TW_RFIX_USAGE_LHEAD] = {
			  GLOF_TW_RFIX_USAGE_LHEAD, GLOF_TW_RFIX_USAGE_LNODE, 
			  GLT_LHEAD_1TON, 
			  "All usage rules of this Root or Fix:"
		  },
		  [GLOF_MW_RFIX_INSTANCE_LHEAD] = {
			  GLOF_MW_RFIX_INSTANCE_LHEAD, GLOF_MW_VOCA_RTINCLUDED_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All usage instances of this Root or Fix:"
		  },
	  },
  },
  [GLT_TW_RFIX] = { 
	  .gie_tbl = { 
		  [GLOF_TW_RFIX_USAGE_LNODE] = {
			  GLOF_TW_RFIX_USAGE_LNODE, GLOF_TW_RFIX_USAGE_LHEAD, 
			  GLT_LNODE_TOP_BACKPT, 
			  "The Root or Fix of this usage rule:"
		  },
		  [GLOF_MW_RFIX_USAGE_INSTANCE_LHEAD] = {
			  GLOF_MW_RFIX_USAGE_INSTANCE_LHEAD, GLOF_TW_VOCA_USAGE_INCLUDED_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All vocab examples of this Roots/Fixes usage instance:"
		  },
	  },
  },
  [GLT_UW_VOCA] = { 
	  .gie_tbl = { 
		  [GLOF_MW_VOCA_TYPEWORDS_LHEAD] = {
			  GLOF_MW_VOCA_TYPEWORDS_LHEAD, GLOF_UW_VOCA_TYPE_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All vocas of this type:"
		  },
		  [GLOF_TW_VOCA_TYPEUSAGE_LHEAD] = {
			  GLOF_TW_VOCA_TYPEUSAGE_LHEAD, GLOF_TW_VOCA_TYPEUSAGE_LNODE, 
			  GLT_LHEAD_1TON, 
			  "All vocab usage examples of this type:"
		  },
	  },
  },
  [GLT_MW_VOCA] = { /* vocab mw */
	  .gie_tbl = { 
		  [GLOF_MW_VOCA_BASEWORDS_LNODE] = {
			  GLOF_MW_VOCA_BASEWORDS_LNODE, GLOF_MW_VOCA_BASEWORDS_LHEAD, 
			  GLT_LNODE_TOP_BACKPT, 
			  "The base word of this vocabulay:"
		  },
		  [GLOF_UW_VOCA_TYPE_LHEAD] = {
			  GLOF_UW_VOCA_TYPE_LHEAD, GLOF_MW_VOCA_TYPEWORDS_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All types to which this vocab belongs:"
		  },
		  [GLOF_SW_VOCA_EXAMPLEV_LHEAD] = {
			  GLOF_SW_VOCA_EXAMPLEV_LHEAD, GLOF_MW_SENT_VUSED_LHEAD, 
			  GLT_LHEAD_1TON, 
			  "All example sentences of this vocabulay:"
		  },
		  [GLOF_MW_VOCA_BASEWORDS_LHEAD] = {
			  GLOF_MW_VOCA_BASEWORDS_LHEAD, GLOF_MW_VOCA_BASEWORDS_LNODE, 
			  GLT_LHEAD_1TON, 
			  "All derivative words of this base word:"
		  },
		  [GLOF_TW_VOCA_MEANINGUSAGE_LHEAD] = {
			  GLOF_TW_VOCA_MEANINGUSAGE_LHEAD, GLOF_TW_VOCA_MEANINGUSAGE_LNODE, 
			  GLT_LHEAD_1TON, 
			  "All meaning or usage info of this vocabulary:"
		  },
		  [GLOF_MW_VOCA_RTINCLUDED_LHEAD] = {
			  GLOF_MW_VOCA_RTINCLUDED_LHEAD, GLOF_MW_RFIX_INSTANCE_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All roots/fixes included in this vocabulary:"
		  },
		  [GLOF_TW_VOCA_WORDSALLG_LHEAD] = {
			  GLOF_TW_VOCA_WORDSALLG_LHEAD, GLOF_MW_GRAM_VWORDS_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All grammar points of this vocabulary:"
		  },
		  [GLOF_TW_VOCA_USAGE_INCLUDED_LHEAD] = {
			  GLOF_TW_VOCA_USAGE_INCLUDED_LHEAD, GLOF_MW_RFIX_USAGE_INSTANCE_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All Roots/Fixes usage rules in this vocabulary:"
		  },
	  },
  },
  [GLT_TW_VOCA] = { /* vocab tw */
	  .gie_tbl = { 
		  [GLOF_TW_VOCA_MEANINGUSAGE_LNODE] = {
			  GLOF_TW_VOCA_MEANINGUSAGE_LNODE, GLOF_TW_VOCA_MEANINGUSAGE_LHEAD, 
			  GLT_LNODE_TOP_BACKPT, 
			  "The vocabulary of this meaning or usage:"
		  },
		  [GLOF_TW_VOCA_TYPEUSAGE_LNODE] = {
			  GLOF_TW_VOCA_TYPEUSAGE_LNODE, GLOF_TW_VOCA_TYPEUSAGE_LHEAD, 
			  GLT_LNODE_BOTTOM_BACKPT, 
			  "The vocab type of this meaning or usage used as:"
		  },
		  [GLOF_TW_VOCA_DUMMY1] = {
			  GLOF_TW_VOCA_DUMMY1, GLOF_TW_VOCA_DUMMY1, 
			  GLT_DUMMY, 
			  "Dummy:"
		  },
		  [GLOF_SW_VOCA_EXAMPLEVUSAGE_LHEAD] = {
			  GLOF_SW_VOCA_EXAMPLEVUSAGE_LHEAD, GLOF_TW_SENT_VUSEDUSAGE_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All sent examples of this usage instance of this vocab:"
		  },
	  },
  },
  [GLT_UW_GRAM] = { /* grammar tw */
	  .gie_tbl = { 
		  [GLOF_MW_GRAM_GTYPE_LHEAD] = {
			  GLOF_MW_GRAM_GTYPE_LHEAD, GLOF_MW_GRAM_GTYPE_LNODE, 
			  GLT_LHEAD_1TON, 
			  "All grammar sub-classes of this big type:"
		  },
	  },
  },
  [GLT_MW_GRAM] = { /* grammar tw */
	  .gie_tbl = { 
		  [GLOF_MW_GRAM_GTYPE_LNODE] = {
			  GLOF_MW_GRAM_GTYPE_LNODE, GLOF_MW_GRAM_GTYPE_LHEAD, 
			  GLT_LNODE_TOP_BACKPT, 
			  "The grammar big type of this sub-class:"
		  },
		  [GLOF_TW_GRAM_GUSAGE_LHEAD] = {
			  GLOF_TW_GRAM_GUSAGE_LHEAD, GLOF_TW_GRAM_GUSAGE_LNODE, 
			  GLT_LHEAD_1TON, 
			  "All small grammar usage points in this sub-class:"
		  },
		  [GLOF_SW_GRAM_EXAMPLEG_LHEAD] = {
			  GLOF_SW_GRAM_EXAMPLEG_LHEAD, GLOF_MW_SENT_GUSED_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All sentence examples for this grammar sub-class:"
		  },
	  },
  },
  [GLT_TW_GRAM] = { /* grammar tw */
	  .gie_tbl = { 
		  [GLOF_TW_GRAM_GUSAGE_LNODE] = {
			  GLOF_TW_GRAM_GUSAGE_LNODE, GLOF_TW_GRAM_GUSAGE_LHEAD, 
			  GLT_LNODE_TOP_BACKPT, 
			  "The grammar sub-class of this small usage point:"
		  },
	  },
	  .gie_tbl = { 
		  [GLOF_MW_GRAM_VWORDS_LHEAD] = {
			  GLOF_MW_GRAM_VWORDS_LHEAD, GLOF_TW_VOCA_WORDSALLG_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All vocabularies this grammar point controls:"
		  },
	  },
	  .gie_tbl = { 
		  [GLOF_SW_GRAM_EXAMPLEUSAGE_LHEAD] = {
			  GLOF_SW_GRAM_EXAMPLEUSAGE_LHEAD, GLOF_TW_SENT_GUSEDUSAGE_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All sent examples of this grammar point:"
		  },
	  },
  },
  [GLT_SENTENCE] = { /* sentence tw */
	  .gie_tbl = { 
		  [GLOF_MW_SENT_VUSED_LHEAD] = {
			  GLOF_MW_SENT_VUSED_LHEAD, GLOF_SW_VOCA_EXAMPLEV_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All key vocabularies used in this sentence:"
		  },
		  [GLOF_TW_SENT_VUSEDUSAGE_LHEAD] = {
			  GLOF_TW_SENT_VUSEDUSAGE_LHEAD, GLOF_SW_VOCA_EXAMPLEVUSAGE_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All vocab detail usage info in this sentence:"
		  },
		  [GLOF_MW_SENT_GUSED_LHEAD] = {
			  GLOF_MW_SENT_GUSED_LHEAD, GLOF_SW_GRAM_EXAMPLEG_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All grammar sub-classes used in this sentence:"
		  },
		  [GLOF_TW_SENT_GUSEDUSAGE_LHEAD] = {
			  GLOF_TW_SENT_GUSEDUSAGE_LHEAD, GLOF_SW_GRAM_EXAMPLEUSAGE_LHEAD, 
			  GLT_LHEAD_MTON, 
			  "All grammar points used in this sentence:"
		  },
	  },
  },
};




