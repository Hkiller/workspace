#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_state_i.h"
#include "plugin_ui_navigation_i.h"

plugin_ui_state_t plugin_ui_state_create(plugin_ui_phase_t phase, const char * state_name) {
    plugin_ui_env_t env = phase->m_env;
    plugin_ui_state_t state;

    if (env->m_backend == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_state_create: env no backend!");
        return NULL;
    }
    
    state = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_state) + env->m_backend->state_capacity);
    if (state == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_state_create: alloc fail!");
        return NULL;
    }

    state->m_phase = phase;
    cpe_str_dup(state->m_name, sizeof(state->m_name), state_name);
    state->m_auto_execute = NULL;
    state->m_packages = NULL;
    
    TAILQ_INIT(&state->m_navigations_to);
    TAILQ_INIT(&state->m_navigations_from);
    TAILQ_INIT(&state->m_navigations_as_loading);
    TAILQ_INIT(&state->m_navigations_as_back);

    cpe_hash_entry_init(&state->m_hh_for_phase);
    if (cpe_hash_table_insert(&phase->m_states, state) != 0) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_state_create: state %s duplicate!", state_name);
        mem_free(env->m_module->m_alloc, state);
        return NULL;
    }

    if (env->m_backend->state_init(env->m_backend->ctx, state) != 0){
        CPE_ERROR(env->m_module->m_em, "plugin_ui_state_create: state %s init fail!", state_name);
        cpe_hash_table_remove_by_ins(&phase->m_states, state);
        mem_free(env->m_module->m_alloc, state);
        return NULL;
    }
    
    return state;
}

void plugin_ui_state_free(plugin_ui_state_t state) {
    plugin_ui_phase_t phase = state->m_phase;
    plugin_ui_env_t env = phase->m_env;

    env->m_backend->state_fini(env->m_backend->ctx, state);

    if (state->m_packages) {
        plugin_package_group_free(state->m_packages);
        state->m_packages = NULL;
    }

    while(!TAILQ_EMPTY(&state->m_navigations_to)) {
        plugin_ui_navigation_free(TAILQ_FIRST(&state->m_navigations_to));
    }

    while(!TAILQ_EMPTY(&state->m_navigations_from)) {
        plugin_ui_navigation_free(TAILQ_FIRST(&state->m_navigations_from));
    }

    while(!TAILQ_EMPTY(&state->m_navigations_as_loading)) {
        plugin_ui_navigation_free(TAILQ_FIRST(&state->m_navigations_as_loading));
    }
    
    while(!TAILQ_EMPTY(&state->m_navigations_as_back)) {
        plugin_ui_navigation_free(TAILQ_FIRST(&state->m_navigations_as_back));
    }
    assert(state->m_auto_execute == NULL);
    
    if (phase->m_init_state == state) phase->m_init_state = NULL;
    if (phase->m_init_call_state == state) phase->m_init_call_state = NULL;
    
    cpe_hash_table_remove_by_ins(&phase->m_states, state);

    mem_free(env->m_module->m_alloc, state);
}

void plugin_ui_state_free_all(const plugin_ui_phase_t phase) {
    struct cpe_hash_it state_it;
    plugin_ui_state_t state;

    cpe_hash_it_init(&state_it, &phase->m_states);

    state = cpe_hash_it_next(&state_it);
    while (state) {
        plugin_ui_state_t next = cpe_hash_it_next(&state_it);
        plugin_ui_state_free(state);
        state = next;
    }
}

plugin_ui_state_t plugin_ui_state_find(plugin_ui_phase_t phase, const char * state_name) {
    struct plugin_ui_state key;
    cpe_str_dup(key.m_name, sizeof(key.m_name), state_name);
    return cpe_hash_table_find(&phase->m_states, &key);
}

const char * plugin_ui_state_name(plugin_ui_state_t state) {
    return state->m_name;
}

plugin_ui_phase_t plugin_ui_state_phase(plugin_ui_state_t state) {
    return state->m_phase;
}

void * plugin_ui_state_data(plugin_ui_state_t state) {
    return state + 1;
}

plugin_package_group_t plugin_ui_state_packages(plugin_ui_state_t state) {
    return state->m_packages;
}

plugin_package_group_t plugin_ui_state_packages_check_create(plugin_ui_state_t state) {
    if (state->m_packages == NULL) {
        state->m_packages = plugin_package_group_create(state->m_phase->m_env->m_module->m_package_module, "");
        if (state->m_packages == NULL) {
            CPE_ERROR(
                state->m_phase->m_env->m_module->m_em,
                "plugin_ui_state_packages_check_create: create group fail!");
            return NULL;
        }
    }

    return state->m_packages;
}

static plugin_ui_navigation_t plugin_ui_state_navitation_to_next(struct plugin_ui_navigation_it * it) {
    plugin_ui_navigation_t * data = (plugin_ui_navigation_t *)(it->m_data);
    plugin_ui_navigation_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_from);
    return r;
}

void plugin_ui_state_navigations_to(plugin_ui_state_t state, plugin_ui_navigation_it_t it) {
    *(plugin_ui_navigation_t *)(it->m_data) = TAILQ_FIRST(&state->m_navigations_to);
    it->next = plugin_ui_state_navitation_to_next;
}

static plugin_ui_navigation_t plugin_ui_state_navitation_from_next(struct plugin_ui_navigation_it * it) {
    plugin_ui_navigation_t * data = (plugin_ui_navigation_t *)(it->m_data);
    plugin_ui_navigation_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_to);
    return r;
}

void plugin_ui_state_navigations_from(plugin_ui_state_t state, plugin_ui_navigation_it_t it) {
    *(plugin_ui_navigation_t *)(it->m_data) = TAILQ_FIRST(&state->m_navigations_from);
    it->next = plugin_ui_state_navitation_from_next;
}

static plugin_ui_navigation_t plugin_ui_state_navitation_as_loading_next(struct plugin_ui_navigation_it * it) {
    plugin_ui_navigation_t * data = (plugin_ui_navigation_t *)(it->m_data);
    plugin_ui_navigation_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_loading);
    return r;
}

void plugin_ui_state_navigations_as_loading(plugin_ui_state_t state, plugin_ui_navigation_it_t it) {
    *(plugin_ui_navigation_t *)(it->m_data) = TAILQ_FIRST(&state->m_navigations_as_loading);
    it->next = plugin_ui_state_navitation_as_loading_next;
}

static plugin_ui_navigation_t plugin_ui_state_navitation_as_back_next(struct plugin_ui_navigation_it * it) {
    plugin_ui_navigation_t * data = (plugin_ui_navigation_t *)(it->m_data);
    plugin_ui_navigation_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_back);
    return r;
}

void plugin_ui_state_navigations_as_back(plugin_ui_state_t state, plugin_ui_navigation_it_t it) {
    *(plugin_ui_navigation_t *)(it->m_data) = TAILQ_FIRST(&state->m_navigations_as_back);
    it->next = plugin_ui_state_navitation_as_back_next;
}

plugin_ui_navigation_t plugin_ui_state_auto_execute(plugin_ui_state_t state) {
    return state->m_auto_execute;
}

void plugin_ui_state_set_auto_execute(plugin_ui_state_t state, plugin_ui_navigation_t navigation) {
    assert(navigation->m_from_state == state);
    state->m_auto_execute = navigation;
}

uint32_t plugin_ui_state_hash(const plugin_ui_state_t state) {
    return cpe_hash_str(state->m_name, strlen(state->m_name));
}

int plugin_ui_state_eq(const plugin_ui_state_t l, const plugin_ui_state_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
