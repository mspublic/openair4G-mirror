
#ifndef __openair_TYPES_H__
#define __openair_TYPES_H__

#ifdef USER_MODE
#include <sys/types.h>
typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
#else
#include <linux/types.h>
#endif




#endif /*__openair_TYPES_H__ */
