#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_component.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"
#include "ui_sprite_chipmunk_obj_load_ctx.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

static uint32_t ui_sprite_chipmunk_obj_body_load_info_id_hash(struct ui_sprite_chipmunk_obj_body_load_info * body_load_info);
static int ui_sprite_chipmunk_obj_body_load_info_id_eq(struct ui_sprite_chipmunk_obj_body_load_info * l, struct ui_sprite_chipmunk_obj_body_load_info * r);

static uint32_t ui_sprite_chipmunk_obj_body_load_info_name_hash(struct ui_sprite_chipmunk_obj_body_load_info * body_load_info);
static int ui_sprite_chipmunk_obj_body_load_info_name_eq(struct ui_sprite_chipmunk_obj_body_load_info * l, struct ui_sprite_chipmunk_obj_body_load_info * r);

static void ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_free(
    struct ui_sprite_chipmunk_obj_load_ctx * ctx, struct ui_sprite_chipmunk_obj_body_load_info * load_info);

int ui_sprite_chipmunk_obj_load_ctx_init(struct ui_sprite_chipmunk_obj_load_ctx * ctx, ui_sprite_chipmunk_obj_t obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(obj);
    
    bzero(ctx, sizeof(*ctx));
    ctx->m_module = obj->m_env->m_module;
    ctx->m_entity = ui_sprite_component_entity(component);
    ctx->m_obj = obj;

    if (cpe_hash_table_init(
            &ctx->m_body_load_infos_by_id,
            ctx->m_module->m_alloc,
            (cpe_hash_fun_t) ui_sprite_chipmunk_obj_body_load_info_id_hash,
            (cpe_hash_eq_t) ui_sprite_chipmunk_obj_body_load_info_id_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_chipmunk_obj_body_load_info, m_hh_for_id),
            -1) != 0)
    {
        return -1;
    }

    if (cpe_hash_table_init(
            &ctx->m_body_load_infos_by_name,
            ctx->m_module->m_alloc,
            (cpe_hash_fun_t) ui_sprite_chipmunk_obj_body_load_info_name_hash,
            (cpe_hash_eq_t) ui_sprite_chipmunk_obj_body_load_info_name_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_chipmunk_obj_body_load_info, m_hh_for_name),
            -1) != 0)
    {
        cpe_hash_table_fini(&ctx->m_body_load_infos_by_id);
        return -1;
    }

    TAILQ_INIT(&ctx->m_body_load_infos);
    TAILQ_INIT(&ctx->m_constraint_infos);
    
    return 0;
}

void ui_sprite_chipmunk_obj_load_ctx_fini(struct ui_sprite_chipmunk_obj_load_ctx * ctx) {
    while(!TAILQ_EMPTY(&ctx->m_constraint_infos)) {
        ui_sprite_chipmunk_obj_constraint_info_t constraint_info = TAILQ_FIRST(&ctx->m_constraint_infos);
        TAILQ_REMOVE(&ctx->m_constraint_infos, constraint_info, m_next_for_ctx);
        mem_free(ctx->m_module->m_alloc, constraint_info);
    }

    while(!TAILQ_EMPTY(&ctx->m_body_load_infos)) {
        ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_free(ctx, TAILQ_FIRST(&ctx->m_body_load_infos));
    }
    cpe_hash_table_fini(&ctx->m_body_load_infos_by_id);
    cpe_hash_table_fini(&ctx->m_body_load_infos_by_name);
}

struct ui_sprite_chipmunk_obj_body_load_info * 
ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_id(struct ui_sprite_chipmunk_obj_load_ctx * ctx, uint32_t id) {
    struct ui_sprite_chipmunk_obj_body_load_info key;
    key.m_body_id = id;
    return (struct ui_sprite_chipmunk_obj_body_load_info * )cpe_hash_table_find(&ctx->m_body_load_infos_by_id, &key);
}

struct ui_sprite_chipmunk_obj_body_load_info * 
ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_name(struct ui_sprite_chipmunk_obj_load_ctx * ctx, const char * name) {
    struct ui_sprite_chipmunk_obj_body_load_info key;
    cpe_str_dup(key.m_body_name, sizeof(key.m_body_name), name);
    return (struct ui_sprite_chipmunk_obj_body_load_info * )cpe_hash_table_find(&ctx->m_body_load_infos_by_name, &key);
}

static int ui_sprite_chipmunk_obj_load_ctx_insert_body_load_info(
    struct ui_sprite_chipmunk_obj_load_ctx * ctx, struct ui_sprite_chipmunk_obj_body_load_info * body_load_info)
{
    struct ui_sprite_chipmunk_obj_body_load_info * new_body_load_info;
    
    new_body_load_info = (struct ui_sprite_chipmunk_obj_body_load_info *)
        mem_alloc(ctx->m_module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_body_load_info));
    if (new_body_load_info == NULL) {
        CPE_ERROR(ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_insert_body_load_info: alloc fail!");
        return -1;
    }
    
    *new_body_load_info = * body_load_info;

    TAILQ_INIT(&new_body_load_info->m_constraints_as_a);
    TAILQ_INIT(&new_body_load_info->m_constraints_as_b);

    if (new_body_load_info->m_body_id) {
        cpe_hash_entry_init(&new_body_load_info->m_hh_for_id);
        if (cpe_hash_table_insert_unique(&ctx->m_body_load_infos_by_id, new_body_load_info) != 0) {
            CPE_ERROR(
                ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_insert_body_load_info: body %s(%d) insert fail, id duplicate!",
                body_load_info->m_body_name, body_load_info->m_body_id);
            mem_free(ctx->m_module->m_alloc, new_body_load_info);
            return -1;
        }
    }
    
    if (new_body_load_info->m_body_name[0]) {
        cpe_hash_entry_init(&new_body_load_info->m_hh_for_name);
        if (cpe_hash_table_insert(&ctx->m_body_load_infos_by_name, new_body_load_info) != 0) {
            CPE_ERROR(
                ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_insert_body_load_info: body %s(%d) insert fail, id duplicate!",
                body_load_info->m_body_name, body_load_info->m_body_id);
            if (new_body_load_info->m_body_id) cpe_hash_table_remove_by_ins(&ctx->m_body_load_infos_by_id, new_body_load_info);
            mem_free(ctx->m_module->m_alloc, new_body_load_info);
            return -1;
        }
    }

    TAILQ_INSERT_TAIL(&ctx->m_body_load_infos, new_body_load_info, m_next);
    
    return 0;
};

int ui_sprite_chipmunk_obj_load_ctx_load_loaded_bodies(struct ui_sprite_chipmunk_obj_load_ctx * ctx) {
    struct ui_sprite_chipmunk_obj_body_it body_it;
    ui_sprite_chipmunk_obj_body_t body;
    
    ui_sprite_chipmunk_obj_bodies(&body_it, ctx->m_obj);

    while((body = ui_sprite_chipmunk_obj_body_it_next(&body_it))) {
        struct ui_sprite_chipmunk_obj_body_load_info body_load_info;

        body_load_info.m_body_id = body->m_id;
        cpe_str_dup(body_load_info.m_body_name, sizeof(body_load_info.m_body_name), body->m_name);
        body_load_info.m_body = body;
        body_load_info.m_body_data = NULL;
        body_load_info.m_state = ui_sprite_chipmunk_obj_body_load_state_loaded;

        if (ui_sprite_chipmunk_obj_load_ctx_insert_body_load_info(ctx, &body_load_info) != 0) return -1;
    }

    return 0;
}

int ui_sprite_chipmunk_obj_load_ctx_load_scene_bodies(struct ui_sprite_chipmunk_obj_load_ctx * ctx, plugin_chipmunk_data_scene_t scene) {
    struct plugin_chipmunk_data_body_it body_it;
    plugin_chipmunk_data_body_t body;
    
    plugin_chipmunk_data_scene_bodies(&body_it, scene);
    while((body = plugin_chipmunk_data_body_it_next(&body_it))) {
        CHIPMUNK_BODY const * body_data = plugin_chipmunk_data_body_data(body);
        struct ui_sprite_chipmunk_obj_body_load_info body_load_info;

        if (body_data->id) {
            if (ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_id(ctx, body_data->id) != NULL) continue;
        }
        else if (body_data->name[0]) {
            if (ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_name(ctx, body_data->name) != NULL) continue;
        }

        body_load_info.m_body_id = body_data->id;
        cpe_str_dup(body_load_info.m_body_name, sizeof(body_load_info.m_body_name), body_data->name);
        body_load_info.m_body = NULL;
        body_load_info.m_body_data = body;
        body_load_info.m_state = ui_sprite_chipmunk_obj_body_load_state_not_load;

        if (ui_sprite_chipmunk_obj_load_ctx_insert_body_load_info(ctx, &body_load_info) != 0) return -1;
    }
    
    return 0;
}

int ui_sprite_chipmunk_obj_load_ctx_load_scene_joins(struct ui_sprite_chipmunk_obj_load_ctx * ctx, plugin_chipmunk_data_scene_t scene) {
    struct plugin_chipmunk_data_constraint_it constraint_it;
    plugin_chipmunk_data_constraint_t constraint;
    
    plugin_chipmunk_data_scene_constraints(&constraint_it, scene);
    while((constraint = plugin_chipmunk_data_constraint_it_next(&constraint_it))) {
        CHIPMUNK_CONSTRAINT const * constraint_data = plugin_chipmunk_data_constraint_data(constraint);
        struct ui_sprite_chipmunk_obj_constraint_info * constraint_info;

        constraint_info = (struct ui_sprite_chipmunk_obj_constraint_info *)
            mem_alloc(ctx->m_module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_constraint_info));
        if (constraint_info == NULL) {
            CPE_ERROR(ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_insert_constraint_info: alloc fail!");
            return -1;
        }

        constraint_info->body_a = ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_id(ctx, constraint_data->body_a);
        if (constraint_info->body_a == NULL) {
            CPE_ERROR(
                ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_insert_constraint_info: body a %d not exist!",
                constraint_data->body_a);
            return -1;
        }

        constraint_info->body_b = ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_id(ctx, constraint_data->body_b);
        if (constraint_info->body_b == NULL) {
            CPE_ERROR(
                ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_insert_constraint_info: body a %d not exist!",
                constraint_data->body_b);
            return -1;
        }

        TAILQ_INSERT_TAIL(&constraint_info->body_a->m_constraints_as_a, constraint_info, m_next_for_a);
        TAILQ_INSERT_TAIL(&constraint_info->body_b->m_constraints_as_b, constraint_info, m_next_for_b);
        TAILQ_INSERT_TAIL(&ctx->m_constraint_infos, constraint_info, m_next_for_ctx);

        constraint_info->m_constraint_data = constraint;
        constraint_info->m_need_process = 0;
    }

    return 0;
}

int ui_sprite_chipmunk_obj_load_ctx_load_tag_load_state_i(
    struct ui_sprite_chipmunk_obj_load_ctx * ctx, struct ui_sprite_chipmunk_obj_body_load_info * body_load_info)
{
    ui_sprite_chipmunk_obj_constraint_info_t constraint_info;

    TAILQ_FOREACH(constraint_info, &body_load_info->m_constraints_as_a, m_next_for_a) {
        if (constraint_info->m_need_process) continue;

        switch(constraint_info->body_b->m_state) {
        case ui_sprite_chipmunk_obj_body_load_state_loaded:
            constraint_info->m_need_process = 1;
            break;
        case ui_sprite_chipmunk_obj_body_load_state_not_load:
            constraint_info->m_need_process = 1;
            constraint_info->body_b->m_state = ui_sprite_chipmunk_obj_body_load_state_need_load;
            if (ui_sprite_chipmunk_obj_load_ctx_load_tag_load_state_i(ctx, constraint_info->body_b) != 0) return -1;
            break;
        case ui_sprite_chipmunk_obj_body_load_state_need_load:
            constraint_info->m_need_process = 1;
            break;
        case ui_sprite_chipmunk_obj_body_load_state_skip:
            break;
        }
    }

    TAILQ_FOREACH(constraint_info, &body_load_info->m_constraints_as_b, m_next_for_b) {
        if (constraint_info->m_need_process) continue;

        switch(constraint_info->body_a->m_state) {
        case ui_sprite_chipmunk_obj_body_load_state_loaded:
            constraint_info->m_need_process = 1;
            break;
        case ui_sprite_chipmunk_obj_body_load_state_not_load:
            constraint_info->m_need_process = 1;
            constraint_info->body_a->m_state = ui_sprite_chipmunk_obj_body_load_state_need_load;
            if (ui_sprite_chipmunk_obj_load_ctx_load_tag_load_state_i(ctx, constraint_info->body_a) != 0) return -1;
            break;
        case ui_sprite_chipmunk_obj_body_load_state_need_load:
            constraint_info->m_need_process = 1;
            break;
        case ui_sprite_chipmunk_obj_body_load_state_skip:
            constraint_info->m_need_process = 0;
            break;
        }
    }
    
    return 0;
}

int ui_sprite_chipmunk_obj_load_ctx_load_tag_load_state(struct ui_sprite_chipmunk_obj_load_ctx * ctx, const char * name) {
    struct ui_sprite_chipmunk_obj_body_load_info * body_load_info;

    body_load_info = ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_name(ctx, name);
    if (body_load_info == NULL) {
        CPE_ERROR(ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_tag_load_state: root body %s not exist!", name);
        return -1;
    }

    switch(body_load_info->m_state) {
    case ui_sprite_chipmunk_obj_body_load_state_loaded:
        CPE_ERROR(ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_tag_load_state: root body %s is already loaded!", name);
        return -1;
    case ui_sprite_chipmunk_obj_body_load_state_not_load:
        break;
    case ui_sprite_chipmunk_obj_body_load_state_need_load:
        return 0;
    case ui_sprite_chipmunk_obj_body_load_state_skip:
        CPE_ERROR(ctx->m_module->m_em, "ui_sprite_chipmunk_obj_load_ctx_tag_load_state: root body %s is skip!", name);
        return -1;
    }

    body_load_info->m_state = ui_sprite_chipmunk_obj_body_load_state_need_load;

    return ui_sprite_chipmunk_obj_load_ctx_load_tag_load_state_i(ctx, body_load_info);
}

void ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_free(
    struct ui_sprite_chipmunk_obj_load_ctx * ctx,
    struct ui_sprite_chipmunk_obj_body_load_info * load_info)
{
    if (load_info->m_body_id) {
        cpe_hash_table_remove_by_ins(&ctx->m_body_load_infos_by_id, load_info);
    }

    if (load_info->m_body_name[0]) {
        cpe_hash_table_remove_by_ins(&ctx->m_body_load_infos_by_name, load_info);
    }

    TAILQ_REMOVE(&ctx->m_body_load_infos, load_info, m_next);
}

uint32_t ui_sprite_chipmunk_obj_body_load_info_id_hash(struct ui_sprite_chipmunk_obj_body_load_info * body_load_info) {
    return body_load_info->m_body_id;
}

int ui_sprite_chipmunk_obj_body_load_info_id_eq(struct ui_sprite_chipmunk_obj_body_load_info * l, struct ui_sprite_chipmunk_obj_body_load_info * r) {
    return l->m_body_id == r->m_body_id;
}

uint32_t ui_sprite_chipmunk_obj_body_load_info_name_hash(struct ui_sprite_chipmunk_obj_body_load_info * body_load_info) {
    return cpe_hash_str(body_load_info->m_body_name, strlen(body_load_info->m_body_name));
}

int ui_sprite_chipmunk_obj_body_load_info_name_eq(struct ui_sprite_chipmunk_obj_body_load_info * l, struct ui_sprite_chipmunk_obj_body_load_info * r) {
    return strcmp(l->m_body_name, r->m_body_name) == 0;
}
