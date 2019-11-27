#ifndef _ENUM_FACTORY_H_
#define _ENUM_FACTORY_H_

#include <stdint.h>
#include "util.h"

// expansion macro for enum value definition
#define ENUM_VALUE(name, assign, hash) name assign,

// expansion macro for enum to string conversion
#define ENUM_CASE(name, assign, hash) case name: return #name;

// expansion macro for string to enum conversion(strcmp version)
#define ENUM_STRCMP(name, assign, hash) if (!strcmp(str,#name)) return name;

// expansion macro for string to enum conversion(str_hash version)
#define ENUM_STRHASH(name, assign, hash) case hash: {/*printf("hash: %d \n", hash); */return name;}

/// declare the access function and define enum values
#define DECLARE_ENUM(EnumType, ENUM_DEF) \
	typedef enum  { \
		ENUM_DEF(ENUM_VALUE) \
	} EnumType  ; \
	const char *get_##EnumType##_string(EnumType dummy); \
	EnumType get_##EnumType##_value(const char *string); \

/// define the access function names
#define DEFINE_ENUM(EnumType, ENUM_DEF, IGNORE_CHCNT) \
	const char *get_##EnumType##_string(EnumType value) \
{ \
	switch(value) \
	{ \
		ENUM_DEF(ENUM_CASE) \
	default: return NULL; /* handle input error */ \
	} \
} \
EnumType get_##EnumType##_value(const char *str) \
{ \
	struct hash_arg ha = {IGNORE_CHCNT, 6, 'A', 26, "_-+=", UINT32_MAX}; \
	unsigned strhash = str_hash(str, &ha); \
	switch(strhash) \
	{ \
		ENUM_DEF(ENUM_STRHASH) \
	default: { printf("hash of %s is %d \n", str, strhash); \
			assert(0); } /* handle input error */ \
	} \
} \

#endif /* _ENUM_FACTORY_H_ */


