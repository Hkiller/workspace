#ifndef CPE_POM_PAGEHEAD_H
#define CPE_POM_PAGEHEAD_H
#include "cpe/pom/pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

#define POM_PAGE_MAGIC ((uint16_t)0x16BC04FE)

struct pom_data_page_head {
    uint16_t m_magic;
    pom_class_id_t m_classId;
    int8_t m_reserve;
    uint16_t m_page_idx;
    uint16_t m_reserve2;
    uint32_t m_obj_per_page;
};

#pragma pack(pop)

#define pom_data_page_head_init(p) do {       \
    struct pom_data_page_head * __p =         \
        (struct pom_data_page_head *)(p);     \
    __p->m_magic = POM_PAGE_MAGIC;            \
    __p->m_classId = POM_INVALID_CLASSID;     \
    __p->m_reserve = 0;                         \
    __p->m_obj_per_page = 0;                    \
    __p->m_page_idx = -1;                       \
} while(0)

#ifdef __cplusplus
}
#endif

#endif
