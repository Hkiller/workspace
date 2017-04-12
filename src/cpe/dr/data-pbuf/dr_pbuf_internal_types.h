#ifndef CPE_DR_PBUF_INTERNAL_TYPES_H
#define CPE_DR_PBUF_INTERNAL_TYPES_H
#include "cpe/dr/dr_types.h"

#define CPE_PBUF_TYPE_VARINT      0 /*Varint: int32,int64,uint32,uint64,sint32,uint64,bool,enum*/
#define CPE_PBUF_TYPE_64BIT       1 /*64-bit: fixed64, sfixed64, double*/
#define CPE_PBUF_TYPE_LENGTH      2 /*Length-delimited: string,bytes,embedded message,packed repeated fields*/
#define CPE_PBUF_TYPE_GROUP_START 3 /*Start group: groups(deprecated)*/
#define CPE_PBUF_TYPE_GROUP_END   4 /*End group: groups(deprecated)*/
#define CPE_PBUF_TYPE_32BIT       5 /*32-bitfixed32,sfixed32,float*/

#endif
