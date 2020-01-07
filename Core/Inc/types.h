/*
 * types.h
 *
 *  Created on: 11 Ã·¿ 2017
 *      Author: gkyr
 */

#ifndef TYPES_H_
#define TYPES_H_

#include "stdint.h"

#define U8 uint8_t
#define S8 int8_t
#define U16 uint16_t
#define S16 int16_t
#define U32 uint32_t
#define S32 int32_t
#define U64 uint64_t
#define S64 int64_t

typedef char BOOL;

//BOOL values
#define TRUE  1
#define FALSE 0

#define _OK  0
#define _ERR 1

#define PACKED __attribute__((packed))


#endif /* TYPES_H_ */
