#ifndef CPE_DR_BSON_INTERNAL_TYPES_H
#define CPE_DR_BSON_INTERNAL_TYPES_H
#include "cpe/dr/dr_types.h"

typedef enum dr_bson_type {
	dr_bson_type_double = 0x01
    , dr_bson_type_string = 0x02
    , dr_bson_type_embeded = 0x03
    , dr_bson_type_array = 0x04
    , dr_bson_type_binary = 0x05
    , dr_bson_type_undefined = 0x06
    , dr_bson_type_oid = 0x07
    , dr_bson_type_boolean = 0x08
    , dr_bson_type_datetime = 0x09
    , dr_bson_type_null = 0x0A
    , dr_bson_type_reg = 0x0B
    , dr_bson_type_dbp = 0x0C /*DBPointer — Deprecated*/
    , dr_bson_type_js = 0x0D
    , dr_bson_type_symbol = 0x0E
    , dr_bson_type_js_w_s = 0x0F
    , dr_bson_type_int32 = 0x10
    , dr_bson_type_timestamp = 0x11 /*int64*/
    , dr_bson_type_int64 = 0x12
    , dr_bson_type_min = 0xFF
    , dr_bson_type_max = 0x7F
} dr_bson_type;

typedef enum dr_bson_sub_type {
    dr_bson_sub_type_binary = 0x00 /*Binary / Generic*/
    , dr_bson_sub_type_function = 0x01 /*Function*/
    , dr_bson_sub_type_binary_old = 0x02
    , dr_bson_sub_type_uuid = 0x03
    , dr_bson_sub_type_md5 = 0x05
    , dr_bson_sub_type_user_defined = 0x80
} dr_bson_sub_type;

#endif
