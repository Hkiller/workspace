#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_percent_decorator.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_search_i.h"
#include "ui_sprite_2d_module_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_data.h"

ui_sprite_2d_search_t
ui_sprite_2d_search_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_SEARCH_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_search_free(ui_sprite_2d_search_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

int ui_sprite_2d_search_set_on_found(ui_sprite_2d_search_t search, const char * on_found) {
    ui_sprite_2d_module_t module = search->m_module;

    if (search->m_on_found) mem_free(module->m_alloc, search->m_on_found);

    if (on_found) {
        search->m_on_found = cpe_str_mem_dup(module->m_alloc, on_found);
        return search->m_on_found == NULL ? -1 : 0;
    }
    else {
        search->m_on_found = NULL;
        return 0;
    }
}

const char * ui_sprite_2d_search_on_found(ui_sprite_2d_search_t search) {
    return search->m_on_found;
}

static void ui_sprite_2d_search_send_event(
    ui_sprite_2d_search_t search,
    ui_sprite_entity_t entity,
    ui_sprite_entity_t found_entity, 
    ui_sprite_2d_transform_t found_entity_transform)
{
    ui_sprite_2d_module_t module = search->m_module;
    UI_SPRITE_2D_SEARCH_RESULT result_data;
    struct dr_data_source data_source[1];
    ui_vector_2 pos;

    if (search->m_on_found == NULL) return;

    result_data.found_entity_id = ui_sprite_entity_id(found_entity);
    cpe_str_dup(result_data.found_entity_name, sizeof(result_data.found_entity_name), ui_sprite_entity_name(found_entity));
    pos = ui_sprite_2d_transform_origin_pos(found_entity_transform);
    result_data.found_entity_pos.x = pos.x;
    result_data.found_entity_pos.y = pos.y;
    data_source[0].m_data.m_meta = module->m_meta_search_result;
    data_source[0].m_data.m_data = &result_data;
    data_source[0].m_data.m_size = sizeof(result_data);
    data_source[0].m_next = NULL;

    ui_sprite_fsm_action_build_and_send_event(
        ui_sprite_fsm_action_from_data(search), search->m_on_found, data_source);
}

int ui_sprite_2d_in_shape(ui_sprite_event_t evt, ui_vector_2 entity_pos, ui_vector_2 target_pos){
    UI_SPRITE_EVT_2D_SEARCH const * evt_data = evt->data;

    if(evt_data->shape.type == UI_SPRITE_2D_SHAPE_BOX){
        UI_SPRITE_2D_SHAPE_DATA_BOX box = evt_data->shape.data.box;
        if(target_pos.x > entity_pos.x + box.lt.x && target_pos.x < entity_pos.x + box.rb.x &&
            target_pos.y > entity_pos.y + box.lt.y && target_pos.y < entity_pos.y + box.rb.y){
            return 0;
        }
    }
    else if(evt_data->shape.type == UI_SPRITE_2D_SHAPE_CIRCLE){
       ui_vector_2 center = UI_VECTOR_2_INITLIZER(evt_data->shape.data.circle.center.x, evt_data->shape.data.circle.center.y);
       if( cpe_math_distance(entity_pos.x - center.x, entity_pos.y - center.y, target_pos.x, target_pos.y) 
           < evt_data->shape.data.circle.radius){
               return 0;
       }
    }
    else if(evt_data->shape.type == UI_SPRITE_2D_SHAPE_SECTOR){
        ui_vector_2 center = UI_VECTOR_2_INITLIZER(evt_data->shape.data.sector.center.x, evt_data->shape.data.sector.center.y);
        if( cpe_math_distance(entity_pos.x - center.x, entity_pos.y - center.y, target_pos.x, target_pos.y) 
            < evt_data->shape.data.circle.radius){
            float angle = cpe_math_angle(entity_pos.x - center.x, entity_pos.y - center.y, target_pos.x, target_pos.y);
            float angle_start = cpe_math_angle_regular(evt_data->shape.data.sector.angle_start);
            if(angle > angle_start && angle < (angle_start + evt_data->shape.data.sector.angle_range)){
                 return 0;
            }
        }
    }

    return -1;
}

struct search_node{
    ui_sprite_entity_t search_entity;
    float distance;
};

int ui_sprite_2d_search_cmp_node_fast(struct search_node * l, struct search_node * r) {
    return l->distance > r->distance ? 1 : l->distance == r->distance ? 0 : -1;
}

int ui_sprite_2d_search_cmp_node_near(struct search_node * l, struct search_node * r) {
    return l->distance < r->distance ? 1 : l->distance == r->distance ? 0 : -1;
}

typedef int (ui_sprite_2d_search_cmp_node_fun_t)(struct search_node * l, struct search_node * r);

static void ui_sprite_2d_search_process(
    ui_sprite_2d_search_t search, ui_sprite_entity_t entity, char * args, ui_sprite_entity_it_t entity_it,
    ui_sprite_event_t evt, ui_sprite_2d_search_cmp_node_fun_t cmp_policy)
{
    ui_sprite_2d_transform_t entity_transform = ui_sprite_2d_transform_find(entity);
    ui_vector_2 entity_pos = ui_sprite_2d_transform_origin_pos(entity_transform);
    ui_sprite_2d_module_t module = search->m_module;
    ui_sprite_entity_t check_entity;
    char * str_require_count;
    int require_count;
    struct search_node node_buf[10];
    struct search_node * nodes = node_buf;
    int node_capacity = CPE_ARRAY_SIZE(node_buf);
    int node_count = 0;
    int i;
    str_require_count = cpe_str_read_and_remove_arg(args, "require-count", ',', '=');
    if (str_require_count == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): search: fast: read arg require-count fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    require_count = atoi(str_require_count);
    if (require_count <= 0 || require_count >= 10240) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): search: fast: require count %d error!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), require_count);
        return;
    }

    if (require_count > node_capacity) {
        nodes = mem_alloc(module->m_alloc, sizeof(struct search_node) * require_count);
        if (nodes == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): search: fast: alloc requilre buf (count=%d) fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), require_count);
            return;
        }
    }

    while((check_entity = ui_sprite_entity_it_next(entity_it))) {
        ui_vector_2 target_pos;
        struct search_node cur_node;
        ui_sprite_2d_transform_t check_transform = ui_sprite_2d_transform_find(check_entity);
        struct search_node * put_node = NULL;

        if (check_transform == NULL) continue;

        target_pos = ui_sprite_2d_transform_origin_pos(check_transform);

        if(ui_sprite_2d_in_shape(evt, entity_pos, target_pos) < 0) continue;

        cur_node.search_entity = check_entity;
        cur_node.distance = cpe_math_distance(entity_pos.x, entity_pos.y, target_pos.x, target_pos.y);

        if (node_count < require_count) {
            put_node = &nodes[node_count++];
        }
        else if (cmp_policy(&nodes[require_count - 1], &cur_node) < 0) {
            put_node = &nodes[require_count - 1];
        }

        if (put_node == NULL) continue;

        *put_node = cur_node;

        while(put_node > nodes) {
            struct search_node * pre_node = put_node - 1;
            struct search_node tmp;

            if (cmp_policy(pre_node, put_node) < 0) break;

            tmp = *put_node;
            *put_node = *pre_node;
            *pre_node = tmp;

            put_node = pre_node;
        }
    }

    for(i=0; i < node_count; i++) {
        ui_sprite_2d_transform_t found_entity_transform = ui_sprite_2d_transform_find(nodes[i].search_entity);
        ui_sprite_2d_search_send_event(search, entity, nodes[i].search_entity, found_entity_transform);
    }

    if (require_count > node_capacity) {
        mem_free(module->m_alloc, nodes);
    }
}

static void ui_sprite_2d_search_do_search_with_entities(
    ui_sprite_2d_search_t search, ui_sprite_entity_t entity, ui_sprite_event_t evt, ui_sprite_entity_it_t entity_it)
{
    UI_SPRITE_EVT_2D_SEARCH const * evt_data = evt->data;
    ui_sprite_2d_module_t module = search->m_module;

    char buf[64];
    char * def;
    char * sep;
    char * args;
    char * name;

    cpe_str_dup(buf, sizeof(buf), evt_data->search_policy);

    def = cpe_str_trim_head(buf);

    if ((sep = strchr(def, ':'))) {
        *sep = 0;
        *cpe_str_trim_tail(sep, def) = 0;

        name = def;
        args = cpe_str_trim_head(sep + 1); 
    }
    else {
        args = cpe_str_trim_tail(def + strlen(def), def);
        *args = 0;
        name = def;
    }

    if (strcmp(name, "fast") == 0) {
        ui_sprite_2d_search_process(search, entity, args, entity_it, evt, ui_sprite_2d_search_cmp_node_fast);
    }
    else if (strcmp(name, "near") == 0) {
        ui_sprite_2d_search_process(search, entity, args, entity_it, evt, ui_sprite_2d_search_cmp_node_near);
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): search: not support search policy %s!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
        return;
    }
} 

static void ui_sprite_2d_search_do_search(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_2d_search_t search = ctx;
    ui_sprite_2d_module_t module = search->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(ctx));
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_entity_it_t entity_it;
    UI_SPRITE_EVT_2D_SEARCH const * evt_data = evt->data;
    if (evt_data->group_name[0]) {
        ui_sprite_group_t group = ui_sprite_group_find_by_name(world, evt_data->group_name);
        if (group == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): search: group %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->group_name);
            return;
        }

        entity_it = ui_sprite_group_entities(module->m_alloc, group);
        if (entity_it == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): search: group %s get entities fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->group_name);
            return;
        }
    }
    else {
        entity_it = ui_sprite_world_entities(module->m_alloc, world);
        if (entity_it == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): search: world get entities fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return;
        }
    }

    ui_sprite_2d_search_do_search_with_entities(search, entity, evt, entity_it);

    ui_sprite_entity_it_free(entity_it);
}

int ui_sprite_2d_search_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_search_t search = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_module_t module = search->m_module;

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_2d_search", ui_sprite_2d_search_do_search, search) != 0)
    {
        CPE_ERROR(module->m_em, "search enter: add eventer handler fail!");
        return -1;
    }	

    return 0;
}

void ui_sprite_2d_search_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_search_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_search_t search = ui_sprite_fsm_action_data(fsm_action);

    bzero(search, sizeof(*search));

    search->m_module = ctx;

    return 0;
}

int ui_sprite_2d_search_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_search_t to_2d_search = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_search_t from_2d_search = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_search_init(to, ctx);

    if (from_2d_search->m_on_found) {
        to_2d_search->m_on_found = cpe_str_mem_dup(module->m_alloc, from_2d_search->m_on_found);
    }

    return 0;
}

void ui_sprite_2d_search_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_search_t search = ui_sprite_fsm_action_data(fsm_action);

    if (search->m_on_found) {
        mem_free(module->m_alloc, search->m_on_found);
        search->m_on_found = NULL;
    }
}

ui_sprite_fsm_action_t ui_sprite_2d_search_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_search_t search = ui_sprite_2d_search_create(fsm_state, name);

	if (search == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_search action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    if (ui_sprite_2d_search_set_on_found(search, cfg_get_string(cfg, "on-found", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create 2d search action: set on-found fail",
            ui_sprite_2d_module_name(module));
        ui_sprite_2d_search_free(search);
        return NULL;
    }

	return ui_sprite_fsm_action_from_data(search);
}

int ui_sprite_2d_search_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_SEARCH_NAME, sizeof(struct ui_sprite_2d_search));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_SEARCH_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_search_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_search_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_search_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_search_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_search_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_SEARCH_NAME, ui_sprite_2d_search_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_2d_search_unregist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_SEARCH_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_SEARCH_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_SEARCH_NAME);
    }
}

const char * UI_SPRITE_2D_SEARCH_NAME = "2d-search";
