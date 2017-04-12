#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/xcalc/xcalc_token.h"
#include "ui_sprite_world_i.h"
#include "ui_sprite_entity_i.h"
#include "ui_sprite_attr_monitor_i.h"

ui_sprite_attr_monitor_t
ui_sprite_attr_monitor_create(
    ui_sprite_world_t world, ui_sprite_attr_monitor_list_t * manage,
    ui_sprite_entity_t entity, ui_sprite_attr_monitor_fun_t fun, void * ctx)
{
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_attr_monitor_t monitor = NULL;

    monitor = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_attr_monitor));
    if (monitor == NULL) {
        CPE_ERROR(repo->m_em, "crate attr monitor: alloc monitor fail!");
        return NULL;
    }

    monitor->m_state = ui_sprite_attr_monitor_idle;
    monitor->m_entity_id = entity->m_id;
    monitor->m_is_triggered = 0;
    monitor->m_fun = fun;
    monitor->m_ctx = ctx;
    monitor->m_manage = manage;
    TAILQ_INIT(&monitor->m_bindings);

    if (monitor->m_manage) {
        TAILQ_INSERT_TAIL(monitor->m_manage, monitor, m_next_for_manage);
    }

    return monitor;
}

void ui_sprite_attr_monitor_free(ui_sprite_world_t world, ui_sprite_attr_monitor_t monitor) {
    ui_sprite_repository_t repo = world->m_repo;

    if (monitor->m_state == ui_sprite_attr_monitor_working) {
        monitor->m_state = ui_sprite_attr_monitor_deleting;
        return;
    }

    assert(monitor->m_state == ui_sprite_attr_monitor_idle);
    if (monitor->m_is_triggered) {
        ui_sprite_attr_monitor_set_triggered(world, monitor, 0);
        assert(monitor->m_is_triggered == 0);
    }

    while(!TAILQ_EMPTY(&monitor->m_bindings)) {
        ui_sprite_attr_monitor_binding_free(world, TAILQ_FIRST(&monitor->m_bindings));
    }

    if (monitor->m_manage) {
        TAILQ_REMOVE(monitor->m_manage, monitor, m_next_for_manage);
    }

    mem_free(repo->m_alloc, monitor);
}

void ui_sprite_attr_monitor_clear_all(ui_sprite_world_t world, ui_sprite_attr_monitor_list_t * manage) {
    while(!TAILQ_EMPTY(manage)) {
        ui_sprite_attr_monitor_free(world, TAILQ_FIRST(manage));
    }
}

int ui_sprite_attr_monitor_bind_attrs(
    ui_sprite_world_t world, ui_sprite_entity_t entity, ui_sprite_attr_monitor_t monitor, const char * input_attrs)
{
    ui_sprite_repository_t repo = world->m_repo;
    char * tmp_attrs;
    char * attrs;
    char * sep;

    tmp_attrs = cpe_str_mem_dup(repo->m_alloc, input_attrs);
    attrs = cpe_str_trim_head(tmp_attrs);
    while((sep = strchr(attrs, ','))) {
        * cpe_str_trim_tail(sep, attrs) = 0;

        if (ui_sprite_attr_monitor_binding_create_to_last_entry(world, entity, monitor, attrs) != 0) {
            mem_free(repo->m_alloc, tmp_attrs);
            return -1;
        }
        
        attrs = cpe_str_trim_head(sep + 1);
    }

    * cpe_str_trim_tail(attrs + strlen(attrs), attrs) = 0;
    if (attrs[0] != 0) {
        if (ui_sprite_attr_monitor_binding_create_to_last_entry(world, entity, monitor, attrs) != 0) {
            mem_free(repo->m_alloc, tmp_attrs);
            return -1;
        }
    }

    mem_free(repo->m_alloc, tmp_attrs);
    return 0;
}

struct ui_sprite_attr_monitor_visit_arg_ctx {
    int m_rv;
    ui_sprite_repository_t m_repo;
    ui_sprite_world_t m_world;
    ui_sprite_entity_t m_entity;
    ui_sprite_attr_monitor_t m_monitor;
};

static void ui_sprite_attr_monitor_on_arg(void * input_ctx, xtoken_t arg) {
    char name_buf[128];
    const char * attr_name;
    struct ui_sprite_attr_monitor_visit_arg_ctx * ctx = input_ctx;
    ui_sprite_attr_monitor_binding_t binding;
    
    attr_name = xtoken_to_str(arg, name_buf, sizeof(name_buf));

    TAILQ_FOREACH(binding, &ctx->m_monitor->m_bindings, m_next_for_binding) {
        if (strcmp(binding->m_name, attr_name) == 0) return;
    }

    if (ui_sprite_attr_monitor_binding_create_to_last_entry(ctx->m_world, ctx->m_entity, ctx->m_monitor, attr_name) != 0) {
        ctx->m_rv = -1;
    }
}

int ui_sprite_attr_monitor_bind_by_def(
    ui_sprite_world_t world, ui_sprite_entity_t entity,
    ui_sprite_attr_monitor_t monitor, const char * def)
{
    struct ui_sprite_attr_monitor_visit_arg_ctx ctx;

    ctx.m_rv = 0;
    ctx.m_repo = world->m_repo;
    ctx.m_world = world;
    ctx.m_entity = entity;
    ctx.m_monitor = monitor;
    
    if (xcomputer_visit_args(ctx.m_repo->m_computer, def, &ctx, ui_sprite_attr_monitor_on_arg) != 0) {
        return -1;
    }

    return ctx.m_rv;
}

void ui_sprite_attr_monitor_set_triggered(ui_sprite_world_t world, ui_sprite_attr_monitor_t monitor, uint8_t is_triggered) {
    if (monitor->m_is_triggered == is_triggered) return;

    if (monitor->m_is_triggered) {
        TAILQ_REMOVE(&world->m_pending_monitors, monitor, m_next_for_pending);
    }

    monitor->m_is_triggered = is_triggered;

    if (monitor->m_is_triggered) {
        TAILQ_INSERT_TAIL(&world->m_pending_monitors, monitor, m_next_for_pending);
    }
}

/**/
static int ui_sprite_attr_monitor_binding_create_to_composite_entry(
    ui_sprite_world_t world, ui_sprite_entity_t entity, ui_sprite_attr_monitor_t monitor, 
    char * attr, size_t attr_capacity, size_t attr_len, LPDRMETAENTRY entry)
{
    ui_sprite_repository_t repo = world->m_repo;
    LPDRMETA meta;
    int entry_num;
    int i;

    if (dr_entry_type(entry) == CPE_DR_TYPE_UNION) {
        CPE_ERROR(
            repo->m_em, "entity %d(%s): create attr monitor bindint to %s: not support union yet!",
            entity->m_id, entity->m_name, attr);
        return -1;
    }

    meta = dr_entry_ref_meta(entry);
    entry_num = dr_meta_entry_num(meta);

    for(i = 0; i < entry_num; ++i) {
        LPDRMETAENTRY child_entry = dr_meta_entry_at(meta, i);
        const char * entry_name = dr_entry_name(child_entry);
        size_t child_entry_len = strlen(entry_name) + 1;

        if (child_entry_len + attr_len > attr_capacity) {
            CPE_ERROR(
                repo->m_em, "entity %d(%s): create attr monitor binding to %s.%s: attr len overflow!",
                entity->m_id, entity->m_name, attr, entry_name);
            return -1;
        }

        snprintf(attr + attr_len, attr_capacity - attr_len, ".%s", entry_name);
        
        if (dr_entry_type(child_entry) > CPE_DR_TYPE_COMPOSITE) {
            if (ui_sprite_attr_monitor_binding_create(world, monitor, attr) == NULL) return -1;
        }
        else {
            if (ui_sprite_attr_monitor_binding_create_to_composite_entry(
                    world, entity, monitor, 
                    attr, attr_capacity, attr_len + child_entry_len, child_entry)
                != 0)
            {
                return -1;
            }
        }
    }

    return 0;
}

int ui_sprite_attr_monitor_binding_create_to_last_entry(
    ui_sprite_world_t world, ui_sprite_entity_t entity,
    ui_sprite_attr_monitor_t monitor, const char * name)
{
    ui_sprite_repository_t repo = world->m_repo;
    struct dr_data_entry attr_buff;
    dr_data_entry_t attr = ui_sprite_entity_find_attr(&attr_buff, entity, name);
    if (attr == NULL) return 0;
    
    if (dr_entry_type(attr->m_entry) > CPE_DR_TYPE_COMPOSITE) {
        if (ui_sprite_attr_monitor_binding_create(world, monitor, name) == NULL) return -1;
    }
    else {
        char buf[128];
        size_t len = strlen(name);
        if (len >= CPE_ARRAY_SIZE(buf)) {
            CPE_ERROR(
                repo->m_em, "entity %d(%s): create attr monitor binding to %s: attr len overflow!",
                entity->m_id, entity->m_name, name);
            return -1;
        }

        memcpy(buf, name, len);
        ui_sprite_attr_monitor_binding_create_to_composite_entry(
            world, entity, monitor,
            buf, CPE_ARRAY_SIZE(buf), len, attr->m_entry);
    }

    return 0;
}

ui_sprite_attr_monitor_binding_t
ui_sprite_attr_monitor_binding_create(ui_sprite_world_t world, ui_sprite_attr_monitor_t monitor, const char * name) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_attr_monitor_binding_t binding;
    size_t name_len = strlen(name) + 1;

    binding = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_attr_monitor_binding) + name_len);
    if (binding == NULL) {
        CPE_ERROR(repo->m_em, "crate event monitor binding %s: alloc fail!", name);
        return NULL;
    }

    memcpy(binding + 1, name, name_len);
    binding->m_monitor = monitor;
    binding->m_name = (char*)(binding + 1);

    cpe_hash_entry_init(&binding->m_hh_for_world);
    if (cpe_hash_table_insert(&world->m_attr_monitor_bindings, binding) != 0) {
        CPE_ERROR(repo->m_em, "crate event monitor binding %s: insert to world fail!", name);
        mem_free(repo->m_alloc, binding);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&monitor->m_bindings, binding, m_next_for_binding);

    return binding;
}

void ui_sprite_attr_monitor_binding_free(ui_sprite_world_t world, ui_sprite_attr_monitor_binding_t binding) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_attr_monitor_t monitor = binding->m_monitor;

    cpe_hash_table_remove_by_ins(&world->m_attr_monitor_bindings, binding);

    TAILQ_REMOVE(&monitor->m_bindings, binding, m_next_for_binding);

    mem_free(repo->m_alloc, binding);
}

uint32_t ui_sprite_attr_monitor_binding_hash(ui_sprite_attr_monitor_binding_t binding) {
    return (cpe_hash_str(binding->m_name, strlen(binding->m_name)) << 8)
        | (binding->m_monitor->m_entity_id & 0xFF);
}

int ui_sprite_attr_monitor_binding_eq(ui_sprite_attr_monitor_binding_t l, ui_sprite_attr_monitor_binding_t r) {
    return l->m_monitor->m_entity_id == r->m_monitor->m_entity_id
        && strcmp(l->m_name, r->m_name) == 0;
}

void ui_sprite_attr_monitor_notify(ui_sprite_world_t world, uint32_t entity_id, const char * attr_name) {
    struct ui_sprite_attr_monitor key_monitor;
    struct ui_sprite_attr_monitor_binding key;
    ui_sprite_attr_monitor_binding_t binding;

    key_monitor.m_entity_id = entity_id;
    key.m_name = attr_name;
    key.m_monitor = &key_monitor;

    binding = cpe_hash_table_find(&world->m_attr_monitor_bindings, &key);

    while(binding) {
        ui_sprite_attr_monitor_set_triggered(world, binding->m_monitor, 1);
        binding = cpe_hash_table_find_next(&world->m_attr_monitor_bindings, binding);
    }
}

void ui_sprite_attr_monitor_process(ui_sprite_world_t world) {

    while(!TAILQ_EMPTY(&world->m_pending_monitors)) {
        ui_sprite_attr_monitor_t monitor = TAILQ_FIRST(&world->m_pending_monitors);

        ui_sprite_attr_monitor_set_triggered(world, monitor, 0);

        assert(monitor->m_state == ui_sprite_attr_monitor_idle);
        monitor->m_state = ui_sprite_attr_monitor_working;

        monitor->m_fun(monitor->m_ctx);

        if (monitor->m_state == ui_sprite_attr_monitor_deleting) {
            monitor->m_state = ui_sprite_attr_monitor_idle;
            ui_sprite_attr_monitor_free(world, monitor);
        }
        else {
            monitor->m_state = ui_sprite_attr_monitor_idle;
        }
    }
}
