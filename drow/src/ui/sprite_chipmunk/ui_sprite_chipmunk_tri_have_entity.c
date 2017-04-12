#include <assert.h>
#include <stdio.h>
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite_tri/ui_sprite_tri_condition.h"
#include "ui/sprite_tri/ui_sprite_tri_condition_meta.h"
#include "ui_sprite_chipmunk_tri_have_entity_i.h"
#include "ui_sprite_chipmunk_tri_scope_i.h"
#include "ui_sprite_chipmunk_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_tri_have_entity_t
ui_sprite_chipmunk_tri_have_entity_create(ui_sprite_tri_rule_t rule) {
    ui_sprite_tri_condition_t condition;
    
    condition = ui_sprite_tri_condition_create(rule, UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY);
    if (condition == NULL) return NULL;

    return (ui_sprite_chipmunk_tri_have_entity_t)ui_sprite_tri_condition_data(condition);
}
    
void ui_sprite_chipmunk_tri_have_entity_free(ui_sprite_chipmunk_tri_have_entity_t have_entity) {
    ui_sprite_tri_condition_t condition;

    condition = ui_sprite_tri_condition_from_data(have_entity);

    ui_sprite_tri_condition_free(condition);
}

ui_sprite_chipmunk_obj_t ui_sprite_chipmunk_tri_have_entity_obj(ui_sprite_chipmunk_tri_have_entity_t have_entity) {
    return have_entity->m_obj;
}

ui_sprite_chipmunk_tri_scope_t ui_sprite_chipmunk_tri_have_entity_scope(ui_sprite_chipmunk_tri_have_entity_t have_entity) {
    return have_entity->m_scope;
}

const char * ui_sprite_chipmunk_tri_have_entity_name_pattern(ui_sprite_chipmunk_tri_have_entity_t have_entity) {
    return have_entity->m_name_pattern;
}
    
int ui_sprite_chipmunk_tri_have_entity_set_name_pattern(ui_sprite_chipmunk_tri_have_entity_t have_entity, const char * pattern) {
    ui_sprite_chipmunk_module_t module = have_entity->m_obj->m_env->m_module;
    pcre2_code * name_re = NULL;
    pcre2_match_data * match_data = NULL;
    char * new_pattern = NULL;
    
    if (pattern) {
        PCRE2_SIZE erroroffset;
        int rv;

        new_pattern = cpe_str_mem_dup(module->m_alloc, pattern);
        if (new_pattern == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_tri_have_entity_init: dup partten %s fail", pattern);
            return -1;
        }
        
        name_re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, 0, &rv, &erroroffset, NULL);
        if (name_re == NULL) {
            PCRE2_UCHAR buf[256];
            pcre2_get_error_message(rv, buf, sizeof(buf));
            CPE_ERROR(
                module->m_em, "ui_sprite_chipmunk_tri_have_entity_init: parce partten %s error at %d: %s(%d)",
                pattern, (int)erroroffset, buf, rv);
            mem_free(module->m_alloc, new_pattern);
            return -1;
        }

        match_data = pcre2_match_data_create_from_pattern(name_re, NULL);
        if (match_data == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_tri_have_entity_init: parce partten %s generate match data error", pattern);
            pcre2_code_free(name_re);
            mem_free(module->m_alloc, new_pattern);
            return -1;
        }
    }

    if (have_entity->m_name_pattern) mem_free(module->m_alloc, have_entity->m_name_pattern);
    if (have_entity->m_name_re) pcre2_code_free(have_entity->m_name_re);
    if (have_entity->m_match_data) pcre2_match_data_free(have_entity->m_match_data);

    have_entity->m_name_pattern = new_pattern;
    have_entity->m_name_re = name_re;
    have_entity->m_match_data = match_data;
    
    return 0;
}

const char * ui_sprite_chipmunk_tri_have_entity_require_count(ui_sprite_chipmunk_tri_have_entity_t have_entity) {
    return have_entity->m_require_count;
}
    
int ui_sprite_chipmunk_tri_have_entity_set_require_count(ui_sprite_chipmunk_tri_have_entity_t have_entity, const char * require_count) {
    ui_sprite_chipmunk_module_t module = have_entity->m_obj->m_env->m_module;
    char * new_require_count = NULL;

    if (require_count) {
        new_require_count = cpe_str_mem_dup(module->m_alloc, require_count);
        if (new_require_count == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_tri_have_entity_init: dup require count %s fail", require_count);
            return -1;
        }
    }

    if (have_entity->m_require_count) mem_free(module->m_alloc, have_entity->m_require_count);

    have_entity->m_require_count = new_require_count;

    return 0;
}

static int ui_sprite_chipmunk_tri_have_entity_init(void * ctx, ui_sprite_tri_condition_t condition) {
    ui_sprite_chipmunk_tri_have_entity_t data = (ui_sprite_chipmunk_tri_have_entity_t)ui_sprite_tri_condition_data(condition);
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    
    data->m_obj = ui_sprite_chipmunk_obj_find(ui_sprite_tri_condition_entity(condition));
    if (data->m_obj == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_tri_have_entity_init: no chipmunk obj!");
        return -1;
    }
    
    data->m_name_pattern = NULL;
    data->m_name_re = NULL;
    data->m_match_data = NULL;
    data->m_require_count = NULL;
    
    data->m_scope = ui_sprite_chipmunk_tri_scope_create(data->m_obj);
    if (data->m_scope == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_tri_have_entity_init: create scope fail!");
        return -1;
    }
    
    return 0;
}

static void ui_sprite_chipmunk_tri_have_entity_fini(void * ctx, ui_sprite_tri_condition_t condition) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_tri_have_entity_t have_entity = (ui_sprite_chipmunk_tri_have_entity_t)ui_sprite_tri_condition_data(condition);
    
    assert(have_entity->m_scope);
    ui_sprite_chipmunk_tri_scope_free(have_entity->m_scope);
    have_entity->m_scope = NULL;

    if (have_entity->m_name_pattern) {
        mem_free(module->m_alloc, have_entity->m_name_pattern);
        have_entity->m_name_pattern = NULL;
    }
    
    if (have_entity->m_name_re) {
        pcre2_code_free(have_entity->m_name_re);
        have_entity->m_name_re = NULL;
    }

    if (have_entity->m_match_data) {
        pcre2_match_data_free(have_entity->m_match_data);
        have_entity->m_match_data = NULL;
    }

    if (have_entity->m_require_count == NULL) {
        mem_free(module->m_alloc, have_entity->m_require_count);
        have_entity->m_require_count = NULL;
    }
}

static int ui_sprite_chipmunk_tri_have_entity_copy(void * ctx, ui_sprite_tri_condition_t condition, ui_sprite_tri_condition_t source) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_tri_have_entity_t to_data = (ui_sprite_chipmunk_tri_have_entity_t)ui_sprite_tri_condition_data(condition);
    ui_sprite_chipmunk_tri_have_entity_t from_data = (ui_sprite_chipmunk_tri_have_entity_t)ui_sprite_tri_condition_data(source);

    if (ui_sprite_chipmunk_tri_have_entity_init(ctx, condition) != 0) return -1;
    
    if (from_data->m_name_pattern) {
        if (ui_sprite_chipmunk_tri_have_entity_set_name_pattern(to_data, from_data->m_name_pattern) != 0) {
            ui_sprite_chipmunk_tri_have_entity_fini(ctx, condition);
            return -1;
        }
    }

    if (from_data->m_require_count) {
        to_data->m_require_count = cpe_str_mem_dup(module->m_alloc, from_data->m_require_count);
    }
    
    return 0;
}

static int ui_sprite_chipmunk_tri_have_entity_check(void * ctx, ui_sprite_tri_condition_t condition, uint8_t * r) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_tri_have_entity_t have_entity = (ui_sprite_chipmunk_tri_have_entity_t)ui_sprite_tri_condition_data(condition);
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(have_entity->m_obj));
    ui_sprite_group_t group = NULL;
    ui_sprite_entity_it_t check_entity_it;
    ui_sprite_entity_t check_entity;
    uint32_t require_count;

    if (have_entity->m_require_count) {
        if (ui_sprite_entity_check_calc_uint32(&require_count, have_entity->m_require_count, entity, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "%s: spine-tri-have-entity check: create group fail",
                ui_sprite_chipmunk_module_name(module));
            return -1;
        }
    }
    else {
        require_count = 1;
    }
    
    group = ui_sprite_group_create(ui_sprite_entity_world(entity), NULL);
    if (group == NULL) {
        CPE_ERROR(
            module->m_em, "%s: spine-tri-have-entity check: create group fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    if (ui_sprite_chipmunk_tri_scope_query_entities(have_entity->m_scope, group) != 0) {
        CPE_ERROR(
            module->m_em, "%s: spine-tri-have-entity check: collect entity fail",
            ui_sprite_chipmunk_module_name(module));
        ui_sprite_group_free(group);
        return -1;
    }

    check_entity_it = ui_sprite_group_entities(module->m_alloc, group);
    if (check_entity_it == NULL) {
        CPE_ERROR(
            module->m_em, "%s: spine-tri-have-entity check: create entity it fail",
            ui_sprite_chipmunk_module_name(module));
        ui_sprite_group_free(group);
        return -1;
    }

    while(require_count > 0 && (check_entity = ui_sprite_entity_it_next(check_entity_it))) {
        if (check_entity == entity) continue;
        
        if (have_entity->m_name_re) {
            const char * entity_name = ui_sprite_entity_name(check_entity);
            int rc = pcre2_match(have_entity->m_name_re, (PCRE2_SPTR)entity_name, strlen(entity_name), 0, 0, have_entity->m_match_data, NULL);
            if (rc >= 0) {
                require_count--;
            }
        }
        else {
            require_count--;
        }
    }
    
    ui_sprite_group_free(group);

    *r = require_count <= 0 ? 1 : 0;
    return 0;
}

int ui_sprite_chipmunk_tri_have_entity_regist(ui_sprite_chipmunk_module_t module) {
    if (module->m_tri) {
        ui_sprite_tri_condition_meta_t meta;

        meta = ui_sprite_tri_condition_meta_create(
            module->m_tri, UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY, sizeof(struct ui_sprite_chipmunk_tri_have_entity),
            module,
            ui_sprite_chipmunk_tri_have_entity_init,
            ui_sprite_chipmunk_tri_have_entity_fini,
            ui_sprite_chipmunk_tri_have_entity_copy,
            ui_sprite_chipmunk_tri_have_entity_check);
        if (meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: spine-tri-have-entity register: meta create fail",
                ui_sprite_chipmunk_module_name(module));
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_chipmunk_tri_have_entity_unregist(ui_sprite_chipmunk_module_t module) {
    if (module->m_tri) {
        ui_sprite_tri_condition_meta_t meta;

        meta = ui_sprite_tri_condition_meta_find(module->m_tri, UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY);
        if (meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: spine-tri-have-entity unregister: meta not exist",
                ui_sprite_chipmunk_module_name(module));
        }
        else {
            ui_sprite_tri_condition_meta_free(meta);
        }
    }
}

const char * UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY = "chipmunk-have-entity";

#ifdef __cplusplus
}
#endif
