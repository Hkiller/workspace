#include <stdlib.h>
#include <assert.h>
#include "cpe/dr/dr_metalib_init.h"
#include "../dr_internal_types.h"

size_t dr_lib_size(LPDRMETALIB pstLib) {
    assert(pstLib);
    return pstLib->m_size;
}

const char *dr_lib_name(LPDRMETALIB a_pstLib) {
    assert(a_pstLib);
    return a_pstLib->m_name;
}

int dr_lib_version(LPDRMETALIB a_pstLib) {
    assert(a_pstLib);
    return a_pstLib->m_version;
}

int dr_lib_build_version(LPDRMETALIB a_pstLib) {
    assert(a_pstLib);
    return a_pstLib->m_build_version;
}

int dr_lib_tag_set_version(LPDRMETALIB a_pstLib) {
    assert(a_pstLib);
    return a_pstLib->m_tag_set_version;
}

LPDRMETALIB dr_lib_attach(void const * p, size_t size) {
    LPDRMETALIB lib = (LPDRMETALIB)p;
    if (p == NULL) return NULL;

    if (size < sizeof(struct tagDRMetaLib)) return NULL;

    if (lib->m_magic != CPE_DR_MAGIC) return NULL;

    if (size < lib->m_size) return NULL;

    return lib;
}
