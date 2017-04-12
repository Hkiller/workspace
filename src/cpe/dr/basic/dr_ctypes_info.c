#include <string.h>
#include "../dr_ctype_ops.h"

struct tagDRCTypeInfo g_dr_ctypeinfos[] = {
    {CPE_DR_TYPE_UNION, "union", -1, -1}
    , {CPE_DR_TYPE_STRUCT, "struct", -1, -1}
    , {CPE_DR_TYPE_CHAR, "char", 1, 1}
    , {CPE_DR_TYPE_UCHAR, "uchar", 1, 1}
    , {CPE_DR_TYPE_INT8, "int8", 1, 1}
    , {CPE_DR_TYPE_INT16, "int16", 2, 2}
    , {CPE_DR_TYPE_UINT16, "uint16", 2, 2}
    , {CPE_DR_TYPE_INT32, "int32", 4, 4}
    , {CPE_DR_TYPE_UINT32, "uint32", 4, 4}
    , {CPE_DR_TYPE_INT32, "int32", 4, 4}
    , {CPE_DR_TYPE_UINT32, "uint32", 4, 4}
    , {CPE_DR_TYPE_INT64, "int64", 8, 8}
    , {CPE_DR_TYPE_UINT64, "uint64", 8, 8}
    , {CPE_DR_TYPE_DATE, "date", 4, 4}
    , {CPE_DR_TYPE_TIME, "time", 4, 4}
    , {CPE_DR_TYPE_DATETIME, "datetime", 8, 8}
    , {CPE_DR_TYPE_MONEY, "money", 4, 4}
    , {CPE_DR_TYPE_FLOAT, "float", 4, 4}
    , {CPE_DR_TYPE_DOUBLE, "double", 8, 8}
    , {CPE_DR_TYPE_IP, "ip", 4, 4}
    , {CPE_DR_TYPE_CHAR, "char", 1, 1}
    , {CPE_DR_TYPE_STRING, "string", -1, 1}
    , {CPE_DR_TYPE_STRING, "string", -1, 1}
    , {CPE_DR_TYPE_VOID, "void", -1, 1}
    , {CPE_DR_TYPE_UINT8, "uint8", 1, 1}
};

static const int g_dr_ctypeinfos_count
= sizeof(g_dr_ctypeinfos) / sizeof(struct tagDRCTypeInfo);

const struct tagDRCTypeInfo *
dr_find_ctype_info_by_name(const char * name) {
    int i = 0;

    for(i = 0; i < g_dr_ctypeinfos_count; ++i) {
        if (strcmp(g_dr_ctypeinfos[i].m_name, name) == 0) {
            return &g_dr_ctypeinfos[i];
        }
    }

    return NULL;
}

const struct tagDRCTypeInfo *
dr_find_ctype_info_by_type(int typeId) {
    if (typeId < 0 || typeId > CPE_DR_TYPE_MAX) {
        return NULL;
    }
    else {
        return &g_dr_ctypeinfos[typeId];
    }
}

const char * dr_type_name(int typeId) {
    if (typeId < 0 || typeId > CPE_DR_TYPE_MAX) {
        return "unknown";
    }
    else {
        return g_dr_ctypeinfos[typeId].m_name;
    }
}

int dr_type_size(int typeId) {
    if (typeId < 0 || typeId > CPE_DR_TYPE_MAX) {
        return -1;
    }
    else {
        return (int)g_dr_ctypeinfos[typeId].m_size;
    }
}

int dr_type_id_from_name(const char * name) {
    const struct tagDRCTypeInfo * info = dr_find_ctype_info_by_name(name);
    return info ? info->m_id : CPE_DR_TYPE_UNKOWN;
}
