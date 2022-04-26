#ifndef _GLUE_ENG_H_
#define _GLUE_ENG_H_

#include "types.h"
#include "enum_factory.h"

/************************************************************************
 * business data types, such as pt/pg/rf/v/g etc.
 */
#define BUSINESS_DATA_TYPES(XX)					\
	XX(BDT_PT, , 409)	/* phoneme transcription */	\
	XX(BDT_PG, , 396)	/* phoneograms */		\
	XX(BDT_RF, , 447)	/* root/fix */			\
	XX(BDT_V, , 21)		/* vocabulary */		\
	XX(BDT_G, , 6)		/* grammar */			\
	XX(BDT_S, , 18)		/* sentence */			\
	XX(BDT_BDTMAX, ,13594383)				\

DECLARE_ENUM(business_data_types, BUSINESS_DATA_TYPES)


/************************************************************************
 * ts types, for indexing XX_uw_list[] in dataroot
 */

/* ts types for phoneme transcription */
#define PHONTRANS_TS_TYPES(XX)							\
	XX(TS_PT_VOWEL, ,9857547)	/* vowel phoneme transcription */	\
	XX(TS_PT_CONSONANT, ,30401449)	/* consonant phoneme transcription */	\
	XX(TS_PT_PTMAX, ,7196719)	/* max of ts pt */			\

DECLARE_ENUM(phontrans_ts_types, PHONTRANS_TS_TYPES)


/* ts types for phonograms */
#define PHONOGRAMS_TS_TYPES(XX)							\
	XX(TS_PG_SINGLE_BASIC, ,217753410)	/* basic single phonograms */	\
	XX(TS_PG_MULTI_BASIC, ,151922421)	/* basic multi phonograms */	\
	XX(TS_PG_TSADVANCED, ,233974286)	/* advanced phonograms */	\
	XX(TS_PG_PGMAX, ,6968231)		/* max of ts pg */		\

DECLARE_ENUM(phonograms_ts_types, PHONOGRAMS_TS_TYPES)


/* ts types for root/fixes */
#define ROOTFIXES_TS_TYPES(XX)					\
	XX(TS_RF_ROOT, ,308639)		/* ROOT */		\
	XX(TS_RF_PREFIX, ,186063147)	/* PREFIX */		\
	XX(TS_RF_SUFFIX, ,223095779)	/* SUFFIX */		\
	XX(TS_RF_RFMAX, ,7864607)	/* max of ts rf */	\

DECLARE_ENUM(rootfixes_ts_types, ROOTFIXES_TS_TYPES)


/* ts types for vocabulary */
#define VOCABULARY_TS_TYPES(XX)				\
	XX(TS_V_PEOPLE, ,180305038)			\
	XX(TS_V_DESCRIBING_PEOPLE, ,37790202)		\
	XX(TS_V_TSFEELINGS_EMOTIONS, ,234062411)	\
	XX(TS_V_HUMAN_BODY, ,92520403)			\
	XX(TS_V_ABOUT_HEALTH, ,717061)			\
	XX(TS_V_AROUND_THE_HOUSE, ,8028517)		\
	XX(TS_V_FOOD_DRINK, ,66052731)			\
	XX(TS_V_LEISURE_TIME, ,132676353)		\
	XX(TS_V_SPORT, ,8499133)			\
	XX(TS_V_THE_MEDIA, ,229023499)			\
	XX(TS_V_TECHNOLOGY, ,227614284)			\
	XX(TS_V_MONEY_MATTERS, ,149206004)		\
	XX(TS_V_TRAVEL_TRANSPORT, ,233529047)		\
	XX(TS_V_EDUCATION_WORK, ,49249323)		\
	XX(TS_V_SOCIETY, ,220303115)			\
	XX(TS_V_OUR_WORLD, ,175792829)			\
	XX(TS_V_ABSTRACT_CONCEPTS, ,786630)		\
	XX(TS_V_VMAX, ,377231)				\

DECLARE_ENUM(vocabulary_ts_types, VOCABULARY_TS_TYPES)


/* ts types  for grammar */
#define GRAMMAR_TS_TYPES(XX)				\
	XX(TS_G_VOCABULARY, ,255941758)			\
	XX(TS_G_SENTENCE, ,215934121)			\
	XX(TS_G_GMAX, ,113591)				\

DECLARE_ENUM(grammar_ts_types, GRAMMAR_TS_TYPES)


/* ts types  for sentence */
#define SENTENCE_TS_TYPES(XX)	\
	XX(TS_S_ALL, ,297)	\
	XX(TS_S_SMAX, ,324503)	\

DECLARE_ENUM(sentence_ts_types, SENTENCE_TS_TYPES)


/* all UWs stored in dataroot.all_uw_idr */
#define UW_TYPES(XX)					\
	XX(UW_PT_VOWEL_SUMMARY, ,256296240)		\
	XX(UW_PT_FRONT_VOWEL, ,67430839)		\
	XX(UW_PT_CNETRAL_VOWEL, ,29787030)		\
	XX(UW_PT_BACK_VOWEL, ,11923848)			\
	XX(UW_PT_HIGH_VOWEL, ,86936188)			\
	XX(UW_PT_MID_VOWEL, ,146299630)			\
	XX(UW_PT_LOW_VOWEL, ,137494054)			\
	XX(UW_PT_LAX_VOWEL, ,131113966)			\
	XX(UW_PT_TENSE_VOWEL, ,227814829)		\
	XX(UW_PT_PURE_VOWEL, ,187662216)		\
	XX(UW_PT_DIPHTHONG, ,39568809)			\
	XX(UW_PT_RCOLOR_VOWEL, ,203151225)		\
							\
	XX(UW_PT_CONS_SUMMARY, ,30401560)		\
	XX(UW_PT_BILABIAL_CONS, ,15730554)		\
	XX(UW_PT_LABIO_DENTAL_CONS, ,130718487)		\
	XX(UW_PT_DENTAL_CONS, ,37713375)		\
	XX(UW_PT_ALVEOLAR_CONS, ,5398911)		\
	XX(UW_PT_POST_ALVEOLAR_CONS, ,184947527)	\
	XX(UW_PT_PALATO_ALVEOLAR_CONS, ,178414484)	\
	XX(UW_PT_PALATAL_CONS, ,178414470)		\
	XX(UW_PT_VELAR_CONS, ,251530580)		\
	XX(UW_PT_GLOTTAL_CONS, ,76574394)		\
	XX(UW_PT_PLOSIVE_CONS, ,183505837)		\
	XX(UW_PT_FRICATIVE_CONS, ,67317451)		\
	XX(UW_PT_AFFRICATE_CONS, ,2384462)		\
	XX(UW_PT_NASAL_CONS, ,154774544)		\
	XX(UW_PT_APPROX_CONS, ,7130159)			\
	XX(UW_PT_TAP_CONS, ,226011513)			\
	XX(UW_PT_GLIDE_CONS, ,76457734)			\
							\
	XX(UW_PG_BASIC_SIGNLE, ,12203222)		\
	XX(UW_PG_BASIC_MULTI, ,12203216)		\
	XX(UW_PG_UWADVANCED, ,247683566)		\
							\
	XX(UW_RF_DUMB_ROOT, ,44995692)			\
	XX(UW_RF_DUMB_PF, ,44995631)			\
	XX(UW_RF_NOUN_SF, ,161216333)			\
	XX(UW_RF_VERB_SF, ,251636741)			\
	XX(UW_RF_ADJ_SF, ,59285)			\
	XX(UW_RF_ADV_SF, ,67397)			\
							\
	XX(UW_VV_AGE, ,160)				\
	XX(UW_VV_LIFE_STAGES, ,134442015)		\
	XX(UW_VV_BABIES_CHILDREN, ,11904482)		\
	XX(UW_VV_DEATH, ,1441733)			\
	XX(UW_VV_FAMILY, ,59623510)			\
	XX(UW_VV_FRIENDS, ,67319125)			\
	XX(UW_VV_LOVE_ROMANCE, ,137465056)		\
	XX(UW_VV_MARRIAGE, ,142887004)			\
							\
	XX(UW_VV_GENERAL_APPEARANCE, ,73347794)		\
	XX(UW_VV_CLOTHES, ,29048582)			\
	XX(UW_VV_TALKING_ABOUT_CLOTHES, ,225946461)	\
	XX(UW_VV_CHARACTER_DESCRIBING, ,26973078)	\
	XX(UW_VV_ADJECTIVES_TO_DESCRIB_PEOPLE, ,1531887)	\
							\
	XX(UW_VV_UWFEELINGS_EMOTIONS, ,247771691)	\
	XX(UW_VV_HAPPY_SAD, ,83444054)			\
	XX(UW_VV_GETTING_ANGRY, ,73463169)		\
	XX(UW_VV_LIKING_DISLIKING, ,134532456)		\
							\
	XX(UW_VV_HEAD_FACE, ,84999694)			\
	XX(UW_VV_HAIR_FACE, ,83321862)			\
	XX(UW_VV_BODY_PARTS, ,18348382)			\
	XX(UW_VV_BODY_MOVEMENTS, ,18348318)		\
	XX(UW_VV_THE_SENSES, ,229027565)			\
							\
	XX(UW_VV_FEELING_ILL, ,61312745)			\
	XX(UW_VV_INJURIES, ,101163850)			\
	XX(UW_VV_AT_THE_DOCTORS, ,9021327)		\
	XX(UW_VV_IN_HOSPITAL, ,101124675)		\
	XX(UW_VV_HEALTHY_LIFESTYLE, ,85005473)		\
							\
	XX(UW_VV_HOUSE_HOMES, ,89931095)			\
	XX(UW_VV_LIVING_ROOM, ,134725792)		\
	XX(UW_VV_KITHEN, ,122808361)			\
	XX(UW_VV_BEDROOM_BATHROOM, ,13773878)		\
	XX(UW_VV_JOBS_AROUND_HOUSE, ,113359809)		\
	XX(UW_VV_PROBLEMS_AROUND_HOUSE, ,186236262)	\
							\
	XX(UW_VV_MEAT_FISH_GROCERIES, ,144417398)	\
	XX(UW_VV_FRUTI_VEGETABLES, ,67540065)		\
	XX(UW_VV_FOOD_IN_TALKING, ,66052857)		\
	XX(UW_VV_COOKING, ,30413461)			\
	XX(UW_VV_EATING_OUT, ,47865200)			\
	XX(UW_VV_DRINKS, ,43562394)			\
							\
	XX(UW_VV_FREE_TIME_IN_TALKING, ,67248982)	\
	XX(UW_VV_HOBBIES_INTERESTS, ,89585760)		\
	XX(UW_VV_ACTIVITIES_INTERESTS, ,1253858)		\
	XX(UW_VV_SPECIAL_OCCASIONS, ,220791272)		\
	XX(UW_VV_FILM_CINEMA, ,63264196)			\
	XX(UW_VV_BOOKS_ART, ,18532332)			\
	XX(UW_VV_MUSIC, ,5847610)			\
							\
	XX(UW_VV_BALL, ,17873)				\
	XX(UW_VV_FOOTBALL, ,66063478)			\
	XX(UW_VV_OTHER_SPORTS, ,175148004)		\
	XX(UW_VV_RESULT_SCORES, ,204141489)		\
							\
	XX(UW_VV_TELEVISION, ,227770642)			\
	XX(UW_VV_NEWSPAPERS, ,156685022)			\
	XX(UW_VV_ADVERTISING, ,1743189)			\
							\
	XX(UW_VV_TELEPHONES, ,227770485)			\
	XX(UW_VV_COMPUTERS, ,30382007)			\
	XX(UW_VV_MACHINE_EQUIPMENT, ,142616617)		\
							\
	XX(UW_VV_MONEY, ,5738692)			\
	XX(UW_VV_RICH_POOR, ,205679488)			\
	XX(UW_VV_AT_THE_BANK, ,9021325)			\
	XX(UW_VV_SHOPS_SHOPPING, ,217320290)		\
							\
	XX(UW_VV_HOLIDAYS, ,89766118)			\
	XX(UW_VV_BEACH_HOLIDAYS, ,13710821)		\
	XX(UW_VV_TRANSPORTS_FORMS, ,233524007)		\
	XX(UW_VV_CARS, ,35612)				\
	XX(UW_VV_DRIVING, ,43567745)			\
	XX(UW_VV_PUBLIC_TRANSPORTS, ,187385382)		\
							\
	XX(UW_VV_SCHOOL, ,214911591)			\
	XX(UW_VV_FURTHER_EDUCATION, ,68858222)		\
	XX(UW_VV_LANGUAGE_LEARNING, ,130928200)		\
	XX(UW_VV_JOBS, ,167692)				\
	XX(UW_VV_EMPLOYMENT, ,53280680)			\
	XX(UW_VV_WORKING_LIFE, ,268093709)		\
	XX(UW_VV_IN_THE_OFFICE, ,101330490)		\
	XX(UW_VV_BUSINESS, ,21343014)			\
							\
	XX(UW_VV_CRIME_PUNISHMENT, ,31680183)		\
	XX(UW_VV_SERIOUS_CRIME, ,215997256)		\
	XX(UW_VV_THEFT_DRUGS_OTHERCRIME, ,229019157)	\
	XX(UW_VV_POLITICS, ,184817550)			\
	XX(UW_VV_RELIGION, ,204010204)			\
	XX(UW_VV_SOCIAL_ISSUES, ,220303003)		\
							\
	XX(UW_VV_ENVIRONMENT, ,53841152)			\
	XX(UW_VV_NATURAL_WORLD, ,154805794)		\
	XX(UW_VV_SCIENCE, ,214922372)			\
	XX(UW_VV_MATERIALS, ,142913610)			\
	XX(UW_VV_HISTORY, ,87155033)			\
	XX(UW_VV_COUNTRIES_NATIONALITIES, ,30521235)	\
	XX(UW_VV_WEATHER, ,263231206)			\
	XX(UW_VV_DISASTERS, ,39616791)			\
	XX(UW_VV_WILD_ANIMALS, ,265241457)		\
	XX(UW_VV_DOMESTIC_FARM_ANIMALS, ,42255895)	\
	XX(UW_VV_ANIMALS_IN_TALKING, ,6089419)		\
	XX(UW_VV_TOWNS_CITIES, ,232539738)		\
							\
	XX(UW_VV_TIME, ,339668)				\
	XX(UW_VV_NUMBERS, ,163809117)			\
	XX(UW_VV_SIMILARITY_DIFFERENCE, ,217737182)	\
	XX(UW_VV_THOUGHTS_IDEAS, ,229204723)		\
	XX(UW_VV_SIZE_SHAPE, ,217963155)			\
	XX(UW_VV_DISTANCE_SPEED, ,39629161)		\
	XX(UW_VV_QUANTITIES, ,199250826)			\
							\
	XX(UW_XX_MAX, ,8135)				\

DECLARE_ENUM(uw_types, UW_TYPES)


/* glue lh node type about */
#define GLUE_LH_NODE_TYPES(XX)							\
	XX(GLT_UW_PTRANS, =(BDT_PT<<GBM_WT_SIZE+WT_UW), 247957918)	\
	XX(GLT_MW_PTRANS, , 152906910)	\
	XX(GLT_TW_PTRANS, , 236076542)	\
	XX(GLT_UW_PGRAMS, =(BDT_PG<<GBM_WT_SIZE+WT_UW), 247949130)	\
	XX(GLT_MW_PGRAMS, , 152898122)	\
	XX(GLT_TW_PGRAMS, , 236067754)	\
	XX(GLT_UW_RFIX, =(BDT_RF<<GBM_WT_SIZE+WT_UW), 247983395)	\
	XX(GLT_MW_RFIX, , 152932387)	\
	XX(GLT_TW_RFIX, , 236102019)	\
	XX(GLT_UW_VOCA, =(BDT_V<<GBM_WT_SIZE+WT_UW), 248059604)	\
	XX(GLT_MW_VOCA, , 153008596)	\
	XX(GLT_TW_VOCA, , 236178228)	\
	XX(GLT_UW_GRAM, =(BDT_G<<GBM_WT_SIZE+WT_UW), 247797952)	\
	XX(GLT_MW_GRAM, , 152746944)	\
	XX(GLT_TW_GRAM, , 235916576)	\
	XX(GLT_MW_SENTENCE, =(BDT_S<<GBM_WT_SIZE+WT_MW), 152949413)	\
	XX(GLT_GLTMAX, , 7196719)	/* max of lh node types */		\

DECLARE_ENUM(glue_lh_node_types, GLUE_LH_NODE_TYPES)

/************************************************************************
 * glue lh node offset options for every kinds of glue lh_node type,
 * no enum magic need
 */
/* phontrans about */
enum { /* phontrans types */
	GLOF_MW_PHONTRANS_TYPE_LHEAD,		/* -y-> */
};
enum { /* concrete phontrans */
	GLOF_UW_PHONTRANS_TYPE_LNODE,		/* <-y- */
	GLOF_TW_PHONTRANS_TUSAGE_LHEAD,		/* -z-> */
};
enum { /* allophones, spelled as, pronunciation rules etc */
	GLOF_MW_PHONTRANS_TUSAGE_LNODE,		/* <-z- */
};

/* phonogram about */
enum { /* three types of phonogram */
	GLOF_MW_PHONOGRAM_TYPE_LHEAD,		/* -w-> */
};
enum { /* concrete phonogram */
	GLOF_UW_PHONOGRAM_TYPE_LNODE,		/* <-w- */
	GLOF_TW_PHONOGRAM_TUSAGE_LHEAD,		/* -x-> */
};
enum { /* sounds of phonogram, spelling rule etc */
	GLOF_MW_PHONOGRAM_TUSAGE_LNODE,		/* <-x- */
};

/* root/fix about */
enum { /* rt/fix classification */
	GLOF_MW_RFIX_TYPE_LHEAD,		/* -q-> */
};
enum { /* concrete rt/fix */
	GLOF_MW_RFIX_TYPE_LNODE,		/* <-q- */
	GLOF_TW_RFIX_USAGE_LHEAD,		/* -r-> */
	GLOF_MW_RFIX_INSTANCE_LHEAD,		/* B1, -s-> */
	GLOF_MW_VOCA_RTINCLUDED_LNODE,		/* <-v- */
};
enum { /* different usage of one rt/fix */
	GLOF_TW_RFIX_USAGE_LNODE,		/* <-r- */
	GLOF_MW_RFIX_USAGE_INSTANCE_LHEAD,	/* C1, -t-> */
	GLOF_TW_VOCA_USAGE_RTINCLUDED_LNODE,	/* <-u- */
};

/* vocabulary about */
enum { /* circumstance, use case etc of a word */
	GLOF_MW_VOCA_CATEWORDS_LHEAD,		/* A1, -i-> */
	GLOF_MW_VOCA_CATE_LNODE,		/* <-k- */
};
enum { /* concrete word */
	GLOF_MW_VOCA_BASEWORDS_LNODE,		/* <-j- */
	GLOF_UW_VOCA_CATE_LHEAD,		/* A2, -k-> */
	GLOF_SW_VOCA_EXAMPLEV_LHEAD,		/* E1, -l-> */
	GLOF_MW_VOCA_BASEWORDS_LHEAD,		/* -j-> */
	GLOF_TW_VOCA_MEANINGUSAGE_LHEAD,	/* -n-> */
	GLOF_MW_RFIX_INSTANCE_LNODE,		/* <-s- */
	GLOF_MW_VOCA_RTINCLUDED_LHEAD,		/* B2, -v-> */
	GLOF_UW_VOCA_CATEWORDS_LNODE,		/* <-i- */
	GLOF_MW_SENT_VUSED_LNODE,		/* <-o- */
};
enum { /* different usage of concrete word */
	GLOF_TW_VOCA_MEANINGUSAGE_LNODE,	/* <-n- */
	GLOF_TW_VOCA_DUMMY1,
	GLOF_SW_VOCA_EXAMPLEVUSAGE_LHEAD,	/* F1, -m-> */
	GLOF_TW_GRAM_VWORDS_LNODE,		/* <-c- */
	GLOF_TW_VOCA_WORDSALLG_LHEAD,		/* D1, -d-> */
	GLOF_TW_SENT_VUSEDUSAGE_LNODE,		/* <-p- */
	GLOF_MW_RFIX_USAGE_INSTANCE_LNODE,	/* <-t- */
	GLOF_TW_VOCA_USAGE_RTINCLUDED_LHEAD,	/* C2, -u-> */
};

/* grammar about */
enum {	/* POF; simple/complex sentences; */
	GLOF_MW_GRAM_GTYPE_LHEAD,		/* -a-> */
};
enum {	/* sub-POF; different complex sentences used as subject or as vt object */
	GLOF_UW_GRAM_GTYPE_LNODE,		/* <-a- */
	GLOF_TW_GRAM_GUSAGE_LHEAD,		/* -b-> */
	GLOF_MW_GRAM_EXAMPLEG_LHEAD,		/* G1, -e-> */
	GLOF_MW_SENT_GUSED_LNODE,		/* <-g- */
};
enum {	/* sub sub-POF; concrete subordinating conjuction, such as because/when etc */
	GLOF_MW_GRAM_GUSAGE_LNODE,		/* <-b- */
	GLOF_TW_GRAM_VWORDS_LHEAD,		/* D2, -c-> */
	GLOF_MW_GRAM_EXAMPLEUSAGE_LHEAD,	/* H1, -f-> */
	GLOF_TW_VOCA_WORDSALLG_LNODE,		/* <-d- */
	GLOF_TW_SENT_GUSEDUSAGE_LNODE,		/* <-h- */
};

/* sentence about */
enum {	/* saved as mw */
	GLOF_MW_SENT_VUSED_LHEAD,		/* E2, -o-> */
	GLOF_TW_SENT_VUSEDUSAGE_LHEAD,		/* F2, -p-> */
	GLOF_MW_VOCA_EXAMPLEV_LNODE,		/* <-l- */
	GLOF_SW_VOCA_EXAMPLEVUSAGE_LNODE,	/* <-m- */
	GLOF_MW_SENT_GUSED_LHEAD,		/* G2, -g-> */
	GLOF_TW_SENT_GUSEDUSAGE_LHEAD,		/* H2, -h-> */
	GLOF_MW_GRAM_EXAMPLEG_LNODE,		/* <-e- */
	GLOF_MW_GRAM_EXAMPLEUSAGE_LNODE,	/* <-f- */
};

/* glue info for english */
extern struct glue_info gi_eng[];

/************************************************************************
 * uw/mw/tw structure for all kinds of data
 */

/* for phoneme transcription */
struct pt_uw_data {
	idt pt_udata_id;
	char *pt_udata_name;
	char *pt_udata_desc;			// verbose description info
};


#endif /* _GLUE_ENG_H_ */
