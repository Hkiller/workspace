#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_2d_part_attr_i.h"
#include "ui_sprite_2d_part_binding_i.h"

static ui_sprite_2d_part_attr_t ui_sprite_2d_part_attr_create_i(ui_sprite_2d_part_t part, const char * name) {
    ui_sprite_2d_module_t module = part->m_transform->m_module;
    ui_sprite_2d_part_attr_t part_attr;

    part_attr = TAILQ_FIRST(&module->m_free_part_attrs);
    if (part_attr == NULL) {
        part_attr = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_2d_part_attr));
        if (part_attr == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_2d_part_attr_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&module->m_free_part_attrs, part_attr, m_next);
    }

    part_attr->m_part = part;
    cpe_str_dup(part_attr->m_name, sizeof(part_attr->m_name), name);
    part_attr->m_inline_value[0] = 0;
    part_attr->m_value = NULL;
    part_attr->m_value_changed = 1;
    part->m_attr_updated = 1;
    
    TAILQ_INSERT_TAIL(&part->m_attrs, part_attr, m_next);
    
    return part_attr;
}

ui_sprite_2d_part_attr_t ui_sprite_2d_part_attr_create(ui_sprite_2d_part_t part, const char * name) {
    if (ui_sprite_2d_part_attr_find(part, name) != NULL) {
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(part->m_transform));
        CPE_ERROR(
            part->m_transform->m_module->m_em,
            "entity %d(%s): create part attr: part %s attr %s already exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), part->m_name, name);
        return NULL;
    }
    else {
        return ui_sprite_2d_part_attr_create_i(part, name);
    }
}

void ui_sprite_2d_part_attr_free(ui_sprite_2d_part_attr_t part_attr) {
    ui_sprite_2d_part_t part = part_attr->m_part;
    ui_sprite_2d_module_t module = part->m_transform->m_module;
    
    assert(part_attr);

    if (part_attr->m_value) {
        mem_free(module->m_alloc, part_attr->m_value);
    }

    part->m_attr_updated = 1;
    
    TAILQ_REMOVE(&part->m_attrs, part_attr, m_next);
    TAILQ_INSERT_TAIL(&module->m_free_part_attrs, part_attr, m_next);
}

void ui_sprite_2d_part_attr_real_free(ui_sprite_2d_module_t module, ui_sprite_2d_part_attr_t part_attr) {
    TAILQ_REMOVE(&module->m_free_part_attrs, part_attr, m_next);
    mem_free(module->m_alloc, part_attr);
}

ui_sprite_2d_part_attr_t ui_sprite_2d_part_attr_find(ui_sprite_2d_part_t part, const char * name) {
    ui_sprite_2d_part_attr_t part_attr;

    assert(name);

    TAILQ_FOREACH(part_attr, &part->m_attrs, m_next) {
        if (strcmp(part_attr->m_name, name) == 0) return part_attr;
    }

    return NULL;
}

const char * ui_sprite_2d_part_attr_name(ui_sprite_2d_part_attr_t part_attr) {
    return part_attr->m_name;
}

const char * ui_sprite_2d_part_attr_value(ui_sprite_2d_part_attr_t part_attr) {
    return part_attr->m_value ? part_attr->m_value : part_attr->m_inline_value;
}

int ui_sprite_2d_part_attr_set_value(ui_sprite_2d_part_attr_t part_attr, const char * value) {
    size_t len;

    if (strcmp(ui_sprite_2d_part_attr_value(part_attr), value) == 0) return 0;

    len = strlen(value);
    if (len + 1 > CPE_ARRAY_SIZE(part_attr->m_inline_value)) {
        ui_sprite_2d_module_t module = part_attr->m_part->m_transform->m_module;
        
        char * new_value = cpe_str_mem_dup(module->m_alloc, value);
        if (new_value == NULL) return -1;
        
        if (part_attr->m_value) {
            mem_free(module->m_alloc, part_attr->m_value);
        }
        part_attr->m_value = new_value;
    }
    else {
        memcpy(part_attr->m_inline_value, value, len);
        part_attr->m_inline_value[len] = 0;
        if (part_attr->m_value) {
            mem_free(part_attr->m_part->m_transform->m_module->m_alloc, part_attr->m_value);
            part_attr->m_value = NULL;
        }
    }

    part_attr->m_value_changed = 1;
    part_attr->m_part->m_attr_updated = 1;
        
    return 0;
}

uint8_t ui_sprite_2d_part_attr_is_value_changed(ui_sprite_2d_part_attr_t part_attr) {
    return part_attr->m_value_changed;
}

ui_sprite_2d_part_t ui_sprite_2d_part_attr_part(ui_sprite_2d_part_attr_t part_attr) {
    return part_attr->m_part;
}

const char * ui_sprite_2d_part_value(ui_sprite_2d_part_t part, const char * attr_name, const char * dft) {
    ui_sprite_2d_part_attr_t part_attr;

    part_attr = ui_sprite_2d_part_attr_find(part, attr_name);

    return part_attr ? ui_sprite_2d_part_attr_value(part_attr) : dft;
}

int ui_sprite_2d_part_set_value(ui_sprite_2d_part_t part, const char * attr_name, const char * value) {
    ui_sprite_2d_part_attr_t part_attr;

    part_attr = ui_sprite_2d_part_attr_find(part, attr_name);
    
    if (value) {
        if (part_attr == NULL) {
            part_attr = ui_sprite_2d_part_attr_create_i(part, attr_name);
            if (part_attr == NULL) return -1;
        }
        
        return ui_sprite_2d_part_attr_set_value(part_attr, value);
    }
    else {
        if (part_attr) ui_sprite_2d_part_attr_free(part_attr);
        return 0;
    }
}

static int ui_sprite_2d_part_bulk_set_value_one(ui_sprite_2d_part_t part, char * begin, char * end, char pair) {
    char * p;
    char * name_end;
    char name_end_buf;
    char * value_end;
    char value_end_buf;
    int rv;
    
    p = strchr(begin, pair);
    if (p == NULL) return -1;

    name_end = cpe_str_trim_tail(p, begin);
    name_end_buf = *name_end;
    *name_end = 0;

    p = cpe_str_trim_head(p + 1);
    value_end = cpe_str_trim_tail(end, p);
    value_end_buf = *value_end;

    rv = ui_sprite_2d_part_set_value(part, begin, p);
    
    *name_end = name_end_buf;
    *value_end = value_end_buf;
    
    return rv;
}
    
int ui_sprite_2d_part_bulk_set_values_mutable(ui_sprite_2d_part_t part, char * defs, char sep, char pair) {
    char * p;
    int rv = 0;
    defs = cpe_str_trim_head(defs);
    
    while((p = strchr(defs, sep))) {
        if (ui_sprite_2d_part_bulk_set_value_one(part, defs, p, pair) != 0) rv = -1;
        defs = cpe_str_trim_head(p + 1);
    }

    p = cpe_str_trim_tail(defs + strlen(defs), defs);
    if (defs != p) {
        if (ui_sprite_2d_part_bulk_set_value_one(part, defs, p, pair) != 0) rv = -1;
    }
    
    return rv;
}

static ui_sprite_2d_part_attr_t ui_sprite_2d_part_attr_next(struct ui_sprite_2d_part_attr_it * it) {
    ui_sprite_2d_part_attr_t * data = (ui_sprite_2d_part_attr_t *)(it->m_data);
    ui_sprite_2d_part_attr_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_sprite_2d_part_attrs(ui_sprite_2d_part_attr_it_t part_attr_it, ui_sprite_2d_part_t part) {
    *(ui_sprite_2d_part_attr_t *)(part_attr_it->m_data) = TAILQ_FIRST(&part->m_attrs);
    part_attr_it->next = ui_sprite_2d_part_attr_next;
}
