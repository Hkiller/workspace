#include <ctype.h>
#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_types.h"
#include "cfg_internal_ops.h"

cfg_t cfg_create(mem_allocrator_t alloc) {
    cfg_manage_t cm;
    size_t allocSize = sizeof(struct cfg_manage);

    cm = (cfg_manage_t)mem_alloc(alloc, allocSize);
    bzero(cm, allocSize);

    cm->m_alloc = alloc;
    cm->m_root.m_type = CPE_CFG_TYPE_STRUCT;
    cm->m_root.m_manage = cm;
    cm->m_root.m_parent = NULL;

    cfg_struct_init(&cm->m_root);

    return (cfg_t)&cm->m_root;
}

void cfg_free(cfg_t cfg) {
    if (cfg == NULL) return;

    assert(cfg->m_manage);

    if ((cfg_t)&cfg->m_manage->m_root == cfg) {
        cfg_struct_fini(&cfg->m_manage->m_root);
        mem_free(
            cfg->m_manage->m_alloc,
            ((char*)cfg) - (sizeof(struct cfg_manage) - sizeof(struct cfg_struct)));
    }
    else {
        assert(cfg->m_parent);

        if (cfg->m_parent->m_type == CPE_CFG_TYPE_STRUCT) {
            cfg_struct_item_delete((struct cfg_struct *)cfg->m_parent, cfg);
        }
        else {
            assert(cfg->m_parent->m_type == CPE_CFG_TYPE_SEQUENCE);
            cfg_seq_item_delete((struct cfg_seq *)cfg->m_parent, cfg);
        }
    }
}

void cfg_fini(cfg_t cfg) {
    switch (cfg->m_type) {
    case CPE_CFG_TYPE_STRUCT:
        cfg_struct_fini((struct cfg_struct *)(cfg));
        break;
    case CPE_CFG_TYPE_SEQUENCE:
        cfg_seq_fini((struct cfg_seq *)(cfg));
        break;
    default:
        //do nothing for other
        break;
    }
}

cfg_t cfg_struct_add_struct(cfg_t s, const char * name, cfg_policy_t policy) {
    return cfg_struct_item_create(
        (struct cfg_struct *)s,
        name,
        CPE_CFG_TYPE_STRUCT,
        sizeof(struct cfg_struct) - sizeof(struct cfg),
        policy);
}

cfg_t cfg_seq_add_struct(cfg_t s) {
    return cfg_seq_item_create(
        (struct cfg_seq *)s,
        CPE_CFG_TYPE_STRUCT,
        sizeof(struct cfg_struct) - sizeof(struct cfg));
}

cfg_t cfg_struct_add_seq(cfg_t s, const char * name, cfg_policy_t policy) {
    return cfg_struct_item_create(
        (struct cfg_struct *)s,
        name,
        CPE_CFG_TYPE_SEQUENCE,
        sizeof(struct cfg_seq) - sizeof(struct cfg),
        policy);
}

cfg_t cfg_seq_add_seq(cfg_t s) {
    return cfg_seq_item_create(
        (struct cfg_seq *)s,
        CPE_CFG_TYPE_SEQUENCE,
        sizeof(struct cfg_seq) - sizeof(struct cfg));
}

cfg_t cfg_struct_add_string(cfg_t s, const char * name, const char * value, cfg_policy_t policy) {
    size_t len = strlen(value) + 1;
    cfg_t rv = cfg_struct_item_create(
        (struct cfg_struct *)s, name, CPE_CFG_TYPE_STRING, len, policy);
    if (rv == NULL) return NULL;
    memcpy(cfg_data(rv), value, len);
    return rv;
}

cfg_t cfg_struct_add_string_len(cfg_t s, const char * name, const char * value, size_t len, cfg_policy_t policy) {
    cfg_t rv = cfg_struct_item_create(
        (struct cfg_struct *)s, name, CPE_CFG_TYPE_STRING, len + 1, policy);
    if (rv == NULL) return NULL;
    memcpy(cfg_data(rv), value, len);
    ((char*)cfg_data(rv))[len] = 0;
    return rv;
}

cfg_t cfg_seq_add_string(cfg_t s, const char * value) {
    size_t len = strlen(value) + 1;
    cfg_t rv = cfg_seq_item_create((struct cfg_seq *)s, CPE_CFG_TYPE_STRING, len);
    if (rv == NULL) return NULL;
    memcpy(cfg_data(rv), value, len);
    return rv;
}

cfg_t cfg_seq_add_string_len(cfg_t s, const char * value, size_t len) {
    cfg_t rv = cfg_seq_item_create((struct cfg_seq *)s, CPE_CFG_TYPE_STRING, len + 1);
    if (rv == NULL) return NULL;
    memcpy(cfg_data(rv), value, len);
((char*)cfg_data(rv))[len] = 0;
    return rv;
}

#define CPE_CFG_GEN_ADD_FUN_TYPE(__type_t, __type, __typeId)      \
cfg_t cfg_struct_add_ ## __type(                                  \
        cfg_t s, const char * name, __type_t v, cfg_policy_t policy) {                \
    cfg_t rv = cfg_struct_item_create(                            \
        (struct cfg_struct *)s, name, __typeId, sizeof(v), policy);            \
    if (rv == NULL) return NULL;                                        \
    *(( __type_t*)cfg_data(rv)) = v;                             \
    return rv;                                                          \
}                                                                       \
cfg_t cfg_seq_add_ ## __type(cfg_t s, __type_t v) {        \
    cfg_t rv = cfg_seq_item_create(                               \
        (struct cfg_seq *)s, __typeId, sizeof(v));                   \
    if (rv == NULL) return NULL;                                        \
    *(( __type_t*)cfg_data(rv)) = v;                             \
    return rv;                                                          \
}

CPE_CFG_GEN_ADD_FUN_TYPE(int8_t, int8, CPE_CFG_TYPE_INT8)
CPE_CFG_GEN_ADD_FUN_TYPE(uint8_t, uint8, CPE_CFG_TYPE_UINT8)
CPE_CFG_GEN_ADD_FUN_TYPE(int16_t, int16, CPE_CFG_TYPE_INT16)
CPE_CFG_GEN_ADD_FUN_TYPE(uint16_t, uint16, CPE_CFG_TYPE_UINT16)
CPE_CFG_GEN_ADD_FUN_TYPE(int32_t, int32, CPE_CFG_TYPE_INT32)
CPE_CFG_GEN_ADD_FUN_TYPE(uint32_t, uint32, CPE_CFG_TYPE_UINT32)
CPE_CFG_GEN_ADD_FUN_TYPE(int64_t, int64, CPE_CFG_TYPE_INT64)
CPE_CFG_GEN_ADD_FUN_TYPE(uint64_t, uint64, CPE_CFG_TYPE_UINT64)
CPE_CFG_GEN_ADD_FUN_TYPE(float, float, CPE_CFG_TYPE_FLOAT)
CPE_CFG_GEN_ADD_FUN_TYPE(double, double, CPE_CFG_TYPE_DOUBLE)

cfg_t cfg_seq_add_value_from_string(cfg_t s, int typeId, const char * value) {
    int capacity;
	cfg_t rv;

    if (typeId == CPE_CFG_TYPE_STRING) return cfg_seq_add_string(s, value);
    if (typeId == CPE_CFG_TYPE_STRUCT) return cfg_seq_add_struct(s);
    if (typeId == CPE_CFG_TYPE_SEQUENCE) return cfg_seq_add_seq(s);

    capacity = dr_type_size(typeId);
    if (capacity <= 0) return NULL;

    rv = cfg_seq_item_create((struct cfg_seq *)s, typeId, capacity);
    if (rv == NULL) return NULL;

    if (dr_ctype_set_from_string(cfg_data(rv), typeId, value, NULL) != 0) {
        cfg_seq_item_delete((struct cfg_seq *)s, rv);
        return NULL;
    }

    return rv;
}

cfg_t cfg_seq_add_value(cfg_t s, int typeId) {
    int capacity;

    if (typeId == CPE_CFG_TYPE_STRUCT) return cfg_seq_add_struct(s);
    if (typeId == CPE_CFG_TYPE_SEQUENCE) return cfg_seq_add_seq(s);

    capacity = dr_type_size(typeId);
    if (capacity <= 0) return NULL;

    return cfg_seq_item_create((struct cfg_seq *)s, typeId, capacity);
}

cfg_t cfg_seq_add_value_from_binary(cfg_t s, int typeId, const void * value) {
    switch(typeId) {
    case CPE_CFG_TYPE_STRUCT:
        return cfg_seq_add_struct(s);
    case CPE_CFG_TYPE_SEQUENCE:
        return cfg_seq_add_seq(s);
    case CPE_DR_TYPE_CHAR:
        return cfg_seq_add_int8(s, * (const int8_t *)value);
    case CPE_DR_TYPE_UCHAR:
        return cfg_seq_add_int8(s, * (const int8_t *)value);
    case CPE_DR_TYPE_INT8:
        return cfg_seq_add_int8(s, * (const int8_t *)value);
    case CPE_DR_TYPE_INT16:
        return cfg_seq_add_int16(s, * (const int16_t *)value);
    case CPE_DR_TYPE_UINT16:
        return cfg_seq_add_int16(s, * (const int16_t *)value);
    case CPE_DR_TYPE_INT32:
        return cfg_seq_add_int32(s, * (const int32_t *)value);
    case CPE_DR_TYPE_UINT32:
        return cfg_seq_add_uint32(s, * (const uint32_t *)value);
    case CPE_DR_TYPE_INT64:
        return cfg_seq_add_int64(s, * (const int64_t *)value);
    case CPE_DR_TYPE_UINT64:
        return cfg_seq_add_uint64(s, * (const uint64_t *)value);
    case CPE_DR_TYPE_DATE:
        return NULL;
    case CPE_DR_TYPE_TIME:
        return NULL;
    case CPE_DR_TYPE_DATETIME:
        return NULL;
    case CPE_DR_TYPE_MONEY:
        return NULL;
    case CPE_DR_TYPE_FLOAT:
        return cfg_seq_add_float(s, * (const float *)value);
    case CPE_DR_TYPE_DOUBLE:
        return cfg_seq_add_double(s, * (const double *)value);
    case CPE_DR_TYPE_IP:
        return NULL;
    case CPE_DR_TYPE_STRING:
        return cfg_seq_add_string(s, (const char *)value);
    case CPE_DR_TYPE_VOID:
        return NULL;
    case CPE_DR_TYPE_UINT8:
        return cfg_seq_add_uint8(s, *(const uint8_t*)value);
    default:
        return NULL;
    }
}

cfg_t cfg_struct_add_value(cfg_t s, const char * name, int typeId, cfg_policy_t policy) {
    int capacity;

    if (typeId == CPE_CFG_TYPE_STRUCT) return cfg_struct_add_struct(s, name, policy);
    if (typeId == CPE_CFG_TYPE_SEQUENCE) return cfg_struct_add_seq(s, name, policy);

    capacity = dr_type_size(typeId);
    if (capacity <= 0) return NULL;

    return cfg_struct_item_create((struct cfg_struct *)s, name, typeId, capacity, policy);
}

cfg_t cfg_struct_add_value_from_string(cfg_t s, const char * name, int typeId, const char * value, cfg_policy_t policy) {
    int capacity;
    cfg_t rv;

    if (typeId == CPE_CFG_TYPE_STRING) return cfg_struct_add_string(s, name, value, policy);
    if (typeId == CPE_CFG_TYPE_STRUCT) return cfg_struct_add_struct(s, name, policy);
    if (typeId == CPE_CFG_TYPE_SEQUENCE) return cfg_struct_add_seq(s, name, policy);

    capacity = dr_type_size(typeId);
    if (capacity <= 0) return NULL;

    rv = cfg_struct_item_create((struct cfg_struct *)s, name, typeId, capacity, policy);
    if (rv == NULL) return NULL;

    if (dr_ctype_set_from_string(cfg_data(rv), typeId, value, NULL) != 0) {
        cfg_struct_item_delete((struct cfg_struct *)s, rv);
        return NULL;
    }

    return rv;
}

cfg_t cfg_struct_add_value_from_binary(cfg_t s, const char * name, int typeId, const void * value, cfg_policy_t policy) {
    switch(typeId) {
    case CPE_CFG_TYPE_STRUCT:
        return cfg_struct_add_struct(s, name, policy);
    case CPE_CFG_TYPE_SEQUENCE:
        return cfg_struct_add_seq(s, name, policy);
    case CPE_DR_TYPE_CHAR:
        return cfg_struct_add_int8(s, name, * (const int8_t *)value, policy);
    case CPE_DR_TYPE_UCHAR:
        return cfg_struct_add_int8(s, name, * (const int8_t *)value, policy);
    case CPE_DR_TYPE_INT8:
        return cfg_struct_add_int8(s, name, * (const int8_t *)value, policy);
    case CPE_DR_TYPE_INT16:
        return cfg_struct_add_int16(s, name, * (const int16_t *)value, policy);
    case CPE_DR_TYPE_UINT16:
        return cfg_struct_add_int16(s, name, * (const int16_t *)value, policy);
    case CPE_DR_TYPE_INT32:
        return cfg_struct_add_int32(s, name, * (const int32_t *)value, policy);
    case CPE_DR_TYPE_UINT32:
        return cfg_struct_add_uint32(s, name, * (const uint32_t *)value, policy);
    case CPE_DR_TYPE_INT64:
        return cfg_struct_add_int64(s, name, * (const int64_t *)value, policy);
    case CPE_DR_TYPE_UINT64:
        return cfg_struct_add_uint64(s, name, * (const uint64_t *)value, policy);
    case CPE_DR_TYPE_DATE:
        return NULL;
    case CPE_DR_TYPE_TIME:
        return NULL;
    case CPE_DR_TYPE_DATETIME:
        return NULL;
    case CPE_DR_TYPE_MONEY:
        return NULL;
    case CPE_DR_TYPE_FLOAT:
        return cfg_struct_add_float(s, name, * (const float *)value, policy);
    case CPE_DR_TYPE_DOUBLE:
        return cfg_struct_add_double(s, name, * (const double *)value, policy);
    case CPE_DR_TYPE_IP:
        return NULL;
    case CPE_DR_TYPE_STRING:
        return cfg_struct_add_string(s, name, (const char *)value, policy);
    case CPE_DR_TYPE_VOID:
        return NULL;
    case CPE_DR_TYPE_UINT8:
        return cfg_struct_add_uint8(s, name, *(const uint8_t*)value, policy);
    default:
        return NULL;
    }
}

cfg_t cfg_struct_add_value_from_string_auto(cfg_t s, const char * name, const char * value, cfg_policy_t policy) {
    float f;
    float d;
    int32_t v32;
	int64_t v64;

    while(value[0] && isspace((unsigned char)value[0])) value++;

    if (value[0] == '-' || value[0] == '.'  || value[0] == '+' || (value[0] >= '0' && value[0] <= '9')) {
        if (dr_ctype_set_from_string(&v32, CPE_DR_TYPE_INT32, value, NULL) == 0) {
            return cfg_struct_add_int32(s, name, v32, policy);
        }

        if (dr_ctype_set_from_string(&v64, CPE_DR_TYPE_INT64, value, NULL) == 0) {
            return cfg_struct_add_int64(s, name, v64, policy);
        }

        if (dr_ctype_set_from_string(&f, CPE_DR_TYPE_FLOAT, value, NULL) == 0) {
            return cfg_struct_add_float(s, name, f, policy);
        }

        if (dr_ctype_set_from_string(&d, CPE_DR_TYPE_DOUBLE, value, NULL) == 0) {
            return cfg_struct_add_double(s, name, d, policy);
        }
    }

    return cfg_struct_add_string(s, name, value, policy);
}

cfg_t cfg_seq_add_value_from_string_auto(cfg_t s, const char * value) {
    float f;
    float d;
    int32_t v32;
    int64_t v64;

    if (value[0] == '-' || value[0] == '.'  || value[0] == '+' || (value[0] >= '0' && value[0] <= '9')) {
        if (dr_ctype_set_from_string(&v32, CPE_DR_TYPE_INT32, value, NULL) == 0) {
            return cfg_seq_add_int32(s, v32);
        }

        if (dr_ctype_set_from_string(&v64, CPE_DR_TYPE_INT64, value, NULL) == 0) {
            return cfg_seq_add_int64(s, v64);
        }

        if (dr_ctype_set_from_string(&f, CPE_DR_TYPE_FLOAT, value, NULL) == 0) {
            return cfg_seq_add_float(s, f);
        }

        if (dr_ctype_set_from_string(&d, CPE_DR_TYPE_DOUBLE, value, NULL) == 0) {
            return cfg_seq_add_double(s, d);
        }
    }

    return cfg_seq_add_string(s, value);
}

