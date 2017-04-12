#include <assert.h>
#include "ui_cache_color_i.h"

ui_cache_color_t ui_cache_color_create(ui_cache_manager_t mgr, const char * name, ui_color_t i_color) {
    ui_cache_color_t color;
    size_t name_len = strlen(name) + 1;
    
    color = mem_alloc(mgr->m_alloc, sizeof(struct ui_cache_color) + name_len);
    if (color == NULL) {
        CPE_ERROR(mgr->m_em, "%s: color: alloc fail!", ui_cache_manager_name(mgr));
    }

    color->m_mgr = mgr;
    color->m_name = (void*)(color + 1);
    color->m_color = *i_color;
    memcpy((void*)color->m_name, name, name_len);

    cpe_hash_entry_init(&color->m_hh);
    if (cpe_hash_table_insert(&mgr->m_colors, color) != 0) {
        CPE_ERROR(mgr->m_em, "%s: color: color %s duplicate!", ui_cache_manager_name(mgr), name);
        mem_free(mgr->m_alloc, color);
        return NULL;
    }

    return color;
}

void ui_cache_color_free(ui_cache_color_t color) {
    ui_cache_manager_t mgr =  color->m_mgr;
    
    cpe_hash_table_remove_by_ins(&mgr->m_colors, color);
    
    mem_free(mgr->m_alloc, color);
}

void ui_cache_color_free_all(ui_cache_manager_t mgr) {
    struct cpe_hash_it color_it;
    ui_cache_color_t color;

    cpe_hash_it_init(&color_it, &mgr->m_colors);

    color = cpe_hash_it_next(&color_it);
    while (color) {
        ui_cache_color_t next = cpe_hash_it_next(&color_it);
        ui_cache_color_free(color);
        color = next;
    }
}

ui_cache_color_t ui_cache_color_find(ui_cache_manager_t mgr, const char * name) {
    struct ui_cache_color key;
    key.m_name = name;
    return cpe_hash_table_find(&mgr->m_colors, &key);
}

int ui_cache_set_color(ui_cache_manager_t mgr, const char * str_color, ui_color_t i_color) {
    ui_cache_color_t color = ui_cache_color_find(mgr, str_color);
    if (color) {
        color->m_color = *i_color;
        return 0;
    }
    else {
        return ui_cache_color_create(mgr, str_color, i_color) ? 0 : -1;
    }
}

int ui_cache_find_color(ui_cache_manager_t mgr, const char * str_color, ui_color_t r_color) {
    ui_cache_color_t color;
    
    if (ui_color_parse_from_str(r_color, str_color) == 0) return 0;

    color = ui_cache_color_find(mgr, str_color);
    if(color) {
        *r_color = color->m_color;
        return 0;
    }
    else { 
        return -1;
    }
}

void ui_cache_get_color(ui_cache_manager_t mgr, const char * str_color, ui_color_t dft_color, ui_color_t r_color) {
    if (ui_cache_find_color(mgr, str_color, r_color) == 0) return;
    *r_color = *dft_color;
}

static struct {
    const char * name;
    ui_color c;
} s_default_colors[] = {
    { "white",   { 1.0f,   1.0f,   1.0f,   1.0f } }
    , { "black", { 0.0f,   0.0f,   0.0f,   1.0f } }
    , { "red",   { 1.0f,   0.0f,   0.0f,   1.0f } }
    , { "green", { 0.0f,   1.0f,   0.0f,   1.0f } }
    , { "blue",  { 0.0f,   0.0f,   1.0f,   1.0f } }
    , { "yellow",{ 1.0f,   1.0f,   0.0f,   1.0f } }
    , { "gray",  { 0.5f,   0.5f,   0.5f,   1.0f } }
    , { "purple",{ 0.63f,  0.63f,  0.63f,  1.0f } }
    , { "golden",{ 1.0f,   0.85f,  0.0f,   1.0f } }
    , { "orange",{ 1.0f,   0.65f,  0.0f,   1.0f } }
    , { "gray", { 0.835f, 0.835f, 0.835f, 1.0f } }
};

int ui_cache_color_load_defaults(ui_cache_manager_t mgr) {
    size_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(s_default_colors); ++i) {
        if (ui_cache_set_color(mgr, s_default_colors[i].name, &s_default_colors[i].c) != 0) return -1;
    }
    
    return 0;
}

uint32_t ui_cache_color_hash(ui_cache_color_t color) {
    return cpe_hash_str(color->m_name, strlen(color->m_name));
}

int ui_cache_color_eq(ui_cache_color_t l, ui_cache_color_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
