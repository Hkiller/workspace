#include "cpe/utils/string_utils.h"
#include "version_svr_version.h"
#include "version_svr_package.h"

version_svr_version_t
version_svr_version_create(version_svr_t svr, const char * name, uint8_t update_strategy) {
    version_svr_version_t version;
    size_t name_len = strlen(name) + 1;
    
    version = mem_calloc(svr->m_alloc, sizeof(struct version_svr_version) + name_len);
    if (version == NULL) {
        CPE_ERROR(svr->m_em, "version_svr_version_create: alloc fail!");
        return NULL;
    }

    version->m_svr = svr;
    version->m_name = (void*)(version + 1);
    version->m_update_strategy = update_strategy;
    TAILQ_INIT(&version->m_packages);
    memcpy((void*)version->m_name, name, name_len);
    
    cpe_hash_entry_init(&version->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_versions_by_str, version) != 0) {
        CPE_ERROR(svr->m_em, "version_svr_version_create: %s duplicate!", name);
        mem_free(svr->m_alloc, version);
        return NULL;
    }
    TAILQ_INSERT_TAIL(&svr->m_versions, version, m_next);

    return version;
}

void version_svr_version_free(version_svr_version_t version) {
    version_svr_t svr = version->m_svr;

    while(!TAILQ_EMPTY(&version->m_packages)) {
        version_svr_package_free(TAILQ_FIRST(&version->m_packages));
    }
    
    cpe_hash_table_remove_by_ins(&svr->m_versions_by_str, version);
    TAILQ_REMOVE(&svr->m_versions, version, m_next);

    mem_free(svr->m_alloc, version);
}

version_svr_version_t version_svr_version_find(version_svr_t svr, const char * str_version) {
    struct version_svr_version key;
    key.m_name = str_version;
    return cpe_hash_table_find(&svr->m_versions_by_str, &key);
}

uint32_t version_svr_version_hash(version_svr_version_t o) {
    return cpe_hash_str(o->m_name, strlen(o->m_name));
}

int version_svr_version_eq(version_svr_version_t l, version_svr_version_t r) {
    return strcmp(l->m_name, r->m_name) == 0 ? 1 : 0;
}
