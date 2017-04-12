#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "appsvr_ad_action_i.h"

appsvr_ad_action_t
appsvr_ad_action_create(appsvr_ad_adapter_t adapter, const char * name, size_t capacity) {
    appsvr_ad_action_t action;

    action = mem_alloc(adapter->m_module->m_alloc, sizeof(struct appsvr_ad_action) + capacity);
    if (action == NULL) {
        CPE_ERROR(adapter->m_module->m_em, "appsvr_ad_action_create: alloc fail!");
        return NULL;
    }

    action->m_module = adapter->m_module;
    action->m_adapter = adapter;
    action->m_name = action->m_name_buf;
    cpe_str_dup(action->m_name_buf, sizeof(action->m_name_buf), name);
    
    TAILQ_INIT(&action->m_requests);

    cpe_hash_entry_init(&action->m_hh);
    if (cpe_hash_table_insert_unique(&adapter->m_module->m_actions, action) != 0) {
        CPE_ERROR(adapter->m_module->m_em, "appsvr_ad_action_create: insert fail!");
        mem_free(adapter->m_module->m_alloc, action);
        return NULL;
    }
    
    TAILQ_INSERT_TAIL(&adapter->m_actions, action, m_next);

    return action;
}

void appsvr_ad_action_free(appsvr_ad_action_t action) {
    cpe_hash_table_remove_by_ins(&action->m_module->m_actions, action);

    assert(action->m_adapter);
    TAILQ_REMOVE(&action->m_adapter->m_actions, action, m_next);

    mem_free(action->m_module->m_alloc, action);
}

void * appsvr_ad_action_data(appsvr_ad_action_t action) {
    return action + 1;
}

appsvr_ad_action_t
appsvr_ad_action_find(appsvr_ad_module_t module, const char * name) {
    struct appsvr_ad_action key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_actions, &key);
}

uint32_t appsvr_ad_action_hash(appsvr_ad_action_t action) {
    return cpe_hash_str(action->m_name, strlen(action->m_name));
}

int appsvr_ad_action_eq(appsvr_ad_action_t l, appsvr_ad_action_t r) {
    return strcmp(l->m_name, r->m_name) == 0 ? 1 : 0;
}
