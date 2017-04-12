#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "render/model/ui_data_action.h"
#include "render/model/ui_data_layout.h"
#include "ui_ed_obj_meta_i.h"
#include "ui_ed_mgr_i.h"

extern char g_metalib_render_model_ed[];

extern ui_ed_obj_t ui_ed_frame_img_load_rel_img(ui_ed_obj_t obj);
extern ui_ed_obj_t ui_ed_obj_create_img_block(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_actor(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_actor_layer(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_actor_frame(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_frame(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_frame_img(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_frame_collision(ui_ed_obj_t parent);

#define ui_ed_obj_meta_init(__src_t, __t, __meta)          \
    obj_meta = &ed_mgr->m_metas[__t - UI_ED_OBJ_TYPE_MIN]; \
    obj_meta->m_type = __t;                                \
    obj_meta->m_src_type = __src_t;                        \
    obj_meta->m_data_meta = __meta;                        \
    assert(obj_meta->m_data_meta)

#define  ui_ed_obj_meta_set_id(__id, __id_set_fun)          \
    obj_meta->m_id_entry = dr_meta_find_entry_by_name(obj_meta->m_data_meta, __id); \
    assert(obj_meta->m_id_entry);                                        \
    obj_meta->m_set_id = (ui_ed_obj_set_id_fun_t )__id_set_fun

#define  ui_ed_obj_meta_set_delete(__delete_fun)          \
    obj_meta->m_delete = (ui_ed_obj_delete_fun_t)__delete_fun

#define  ui_ed_obj_meta_set_child(__ct, __fun) \
    obj_meta->m_child_create[__ct - UI_ED_OBJ_TYPE_MIN] = __fun

#define  ui_ed_obj_meta_set_rel(__rel_type, __load_fun) \
    obj_meta->m_rels[__rel_type - UI_ED_REL_TYPE_MIN].m_type = __rel_type; \
    obj_meta->m_rels[__rel_type - UI_ED_REL_TYPE_MIN].m_load = __load_fun

void ui_ed_obj_mgr_init_metas(ui_ed_mgr_t ed_mgr) {
    ui_ed_obj_meta_t obj_meta;

    bzero(ed_mgr->m_metas, sizeof(ed_mgr->m_metas));

    /*src*/
    ui_ed_obj_meta_init(0, ui_ed_obj_type_src, dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_render_model_ed, "ui_ed_src"));
    ui_ed_obj_meta_set_child(ui_ed_obj_type_actor, ui_ed_obj_create_actor);
    ui_ed_obj_meta_set_child(ui_ed_obj_type_frame, ui_ed_obj_create_frame);
    ui_ed_obj_meta_set_child(ui_ed_obj_type_img_block, ui_ed_obj_create_img_block);

    /*img_block*/
    ui_ed_obj_meta_init(ui_data_src_type_module, ui_ed_obj_type_img_block, ui_data_img_block_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_id("id", ui_data_img_block_set_id);
    ui_ed_obj_meta_set_delete(ui_data_img_block_free);

    /*frame*/
    ui_ed_obj_meta_init(ui_data_src_type_sprite, ui_ed_obj_type_frame, ui_data_frame_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_child(ui_ed_obj_type_frame_img, ui_ed_obj_create_frame_img);
    ui_ed_obj_meta_set_child(ui_ed_obj_type_frame_collision, ui_ed_obj_create_frame_collision);
    ui_ed_obj_meta_set_id("id", ui_data_frame_set_id);
    ui_ed_obj_meta_set_delete(ui_data_frame_free);

    /*frame_img*/
    ui_ed_obj_meta_init(ui_data_src_type_sprite, ui_ed_obj_type_frame_img, ui_data_frame_img_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_rel(ui_ed_rel_type_use_img, ui_ed_frame_img_load_rel_img);
    ui_ed_obj_meta_set_delete(ui_data_frame_img_free);

    /*frame_collision*/
    ui_ed_obj_meta_init(ui_data_src_type_sprite, ui_ed_obj_type_frame_collision, ui_data_frame_collision_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_delete(ui_data_frame_collision_free);

    /*actor*/
    ui_ed_obj_meta_init(ui_data_src_type_action, ui_ed_obj_type_actor, ui_data_actor_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_id("id", ui_data_actor_set_id);
    ui_ed_obj_meta_set_delete(ui_data_actor_free);
    ui_ed_obj_meta_set_child(ui_ed_obj_type_actor_layer, ui_ed_obj_create_actor_layer);

    /*actor_layer*/
    ui_ed_obj_meta_init(ui_data_src_type_action, ui_ed_obj_type_actor_layer, ui_data_actor_layer_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_delete(ui_data_actor_layer_free);
    ui_ed_obj_meta_set_child(ui_ed_obj_type_actor_frame, ui_ed_obj_create_actor_frame);

    /*actor_frame*/
    ui_ed_obj_meta_init(ui_data_src_type_action, ui_ed_obj_type_actor_frame, ui_data_actor_frame_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_delete(ui_data_actor_frame_free);
}

void ui_ed_obj_mgr_fini_metas(ui_ed_mgr_t ed_mgr) {
    uint8_t i;

    for(i = UI_ED_OBJ_TYPE_MIN; i < UI_ED_OBJ_TYPE_MAX; ++i) {
        ui_ed_obj_meta_t obj_meta = &ed_mgr->m_metas[i - UI_ED_OBJ_TYPE_MIN];
        bzero(obj_meta, sizeof(*obj_meta));
    }
}

int ui_ed_mgr_register_obj_type(
    ui_ed_mgr_t mgr, ui_data_src_type_t src_type, ui_ed_obj_type_t obj_type, LPDRMETA meta,
    ui_ed_obj_delete_fun_t delete_fun,
    const char * id_entry_name, ui_ed_obj_set_id_fun_t id_set_fun)
{
    ui_ed_obj_meta_t obj_meta;

    if (obj_type < UI_ED_OBJ_TYPE_MIN || obj_type >= UI_ED_OBJ_TYPE_MAX) {
        CPE_ERROR(mgr->m_em, "%s: register type %d: type overflow!", ui_ed_mgr_name(mgr), obj_type);
        return -1;
    }

    obj_meta = &mgr->m_metas[obj_type - UI_ED_OBJ_TYPE_MIN];
    obj_meta->m_type = obj_type;
    obj_meta->m_src_type = src_type;
    obj_meta->m_data_meta = meta;
    obj_meta->m_delete = delete_fun,
    assert(obj_meta->m_data_meta);

    if (id_entry_name) {
        assert(id_set_fun);
        obj_meta->m_id_entry = dr_meta_find_entry_by_name(obj_meta->m_data_meta, id_entry_name);
        assert(obj_meta->m_id_entry);
        obj_meta->m_set_id = id_set_fun;
    }

    return 0;
}

int ui_ed_mgr_register_obj_child(
    ui_ed_mgr_t mgr, ui_ed_obj_type_t parent_obj_type, ui_ed_obj_type_t child_obj_type, ui_ed_obj_create_fun_t create_fun)
{
    if (parent_obj_type < UI_ED_OBJ_TYPE_MIN || parent_obj_type >= UI_ED_OBJ_TYPE_MAX) {
        CPE_ERROR(
            mgr->m_em, "%s: register child %d-%d: parent type overflow!",
            ui_ed_mgr_name(mgr), parent_obj_type, child_obj_type);
        return -1;
    }

    if (child_obj_type < UI_ED_OBJ_TYPE_MIN || child_obj_type >= UI_ED_OBJ_TYPE_MAX) {
        CPE_ERROR(
            mgr->m_em, "%s: register child %d-%d: child type overflow!",
            ui_ed_mgr_name(mgr), child_obj_type, child_obj_type);
        return -1;
    }

    mgr->m_metas[parent_obj_type - UI_ED_OBJ_TYPE_MIN]
        .m_child_create[child_obj_type - UI_ED_OBJ_TYPE_MIN] = create_fun;

    return 0;
}

int ui_ed_mgr_unregister_obj_type(ui_ed_mgr_t mgr, ui_ed_obj_type_t obj_type) {
    uint8_t i;
    ui_ed_obj_meta_t obj_meta;

    if (obj_type < UI_ED_OBJ_TYPE_MIN || obj_type >= UI_ED_OBJ_TYPE_MAX) {
        CPE_ERROR(
            mgr->m_em, "%s: unregister obj type %d: parent type overflow!",
            ui_ed_mgr_name(mgr), obj_type);
        return -1;
    }

    ui_ed_mgr_free_obj_by_type(mgr, obj_type);

    for(i = 0; i < CPE_ARRAY_SIZE(mgr->m_metas); ++i) {
        mgr->m_metas[i].m_child_create[obj_type - UI_ED_OBJ_TYPE_MIN] = NULL;
    }

    obj_meta = &mgr->m_metas[obj_type - UI_ED_OBJ_TYPE_MIN];
    bzero(obj_meta, sizeof(*obj_meta));
    return 0;
}
