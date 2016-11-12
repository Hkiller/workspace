#include <assert.h>
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/utils/id_generator.h"

gd_id_generator_t
gd_id_generator_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;
    gd_id_generator_t gen;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL) return NULL;

    gen = (gd_id_generator_t)nm_node_data(node);
    if (gen->magic != GD_ID_GENERATOR_MAGIC) return NULL;

    return gen;
}

gd_id_generator_t
gd_id_generator_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;
    gd_id_generator_t gen;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL) return NULL;

    gen = (gd_id_generator_t)nm_node_data(node);
    if (gen->magic != GD_ID_GENERATOR_MAGIC) return NULL;

    return gen;
}

const char * gd_id_generator_name(gd_id_generator_t generator) {
    return nm_node_name(nm_node_from_data(generator));
}

cpe_hash_string_t
gd_id_generator_name_hs(gd_id_generator_t generator) {
    return nm_node_name_hs(nm_node_from_data(generator));
}

gd_app_context_t gd_id_generator_app(gd_id_generator_t generator) {
    return generator->app;
}

int gd_id_generator_generate(gd_id_t * r, gd_id_generator_t generator, const char * id_name) {
    assert(generator);
    assert(generator->magic == GD_ID_GENERATOR_MAGIC);
    assert(generator->gen_fun);

    return generator->gen_fun(r, generator, id_name);
}

