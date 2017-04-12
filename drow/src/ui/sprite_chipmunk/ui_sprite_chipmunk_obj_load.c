#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/math_ex.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_shape_group_i.h"
#include "ui_sprite_chipmunk_load_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_chipmunk_obj_load(void * ctx, ui_sprite_component_t comp, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(comp);
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(comp);
    ui_sprite_chipmunk_env_t env = chipmunk_obj->m_env;
    struct cfg_it bodies_it;
    cfg_t body_cfg;
    const char * str_value;

    cfg_it_init(&bodies_it, cfg_find_cfg(cfg, "bodies"));
    while((body_cfg = cfg_it_next(&bodies_it))) {
        if ((str_value = cfg_get_string(body_cfg, "load-from", NULL))) {
            const char * sep;

            if ((sep = strrchr(str_value, '#'))) {
                char path[128];
                ui_sprite_chipmunk_obj_body_t body = NULL;
                size_t len = sep - str_value;
                ui_data_src_t chipmunk_src;
                plugin_chipmunk_data_scene_t scene;;
                plugin_chipmunk_data_body_t data_body;

                if (len + 1 > CPE_ARRAY_SIZE(path)) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: load from %s: path overflow!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
                    return -1;
                }

                memcpy(path, str_value, len);
                path[len] = 0;

                chipmunk_src = ui_data_src_find_by_path(
                    plugin_chipmunk_module_data_mgr(module->m_chipmunk_module), path, ui_data_src_type_chipmunk_scene);
                if (chipmunk_src == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: find chipmunk scene %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), path);
                    return -1;
                }

                scene = (plugin_chipmunk_data_scene_t)ui_data_src_product(chipmunk_src);
                if (scene == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: chipmunk scene %s not loaded!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), path);
                    return -1;
                }

                data_body = plugin_chipmunk_data_body_find_by_name(scene, sep + 1);
                if (data_body == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: chipmunk scene %s body %s not exist!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), path, sep + 1);
                    return -1;
                }

                body = ui_sprite_chipmunk_obj_body_create_from_data(chipmunk_obj, data_body, 0);
                if (body == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: load from %s: create body from data fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
                    return -1;
                }

                if (ui_sprite_chipmunk_load_body_attrs(body->m_obj->m_env, &body->m_body_attrs, body_cfg) != 0) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: load from %s: load body detail fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
                    return -1;
                }
            }
            else {
                ui_sprite_chipmunk_obj_body_t body = NULL;
                
                body = ui_sprite_chipmunk_obj_body_create(chipmunk_obj, 0, sep + 1, 0);
                if (body == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: load from %s: create body fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
                    return -1;
                }
            }
        }
        else {
            ui_sprite_chipmunk_obj_body_t body = NULL;
            struct cfg_it shapes_it;
            cfg_t shape_cfg;
            uint32_t id = cfg_get_uint32(body_cfg, "id", 0);
            const char * name = cfg_get_string(body_cfg, "name", "");

            body = ui_sprite_chipmunk_obj_body_create(chipmunk_obj, id, name, 0);
            if (body == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk obj load: create body '%s' fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
                return -1;
            }

            cfg_it_init(&shapes_it, cfg_find_cfg(body_cfg, "shapes"));
            while((shape_cfg = cfg_it_next(&shapes_it))) {
                CHIPMUNK_FIXTURE * fixture_data;
                ui_sprite_chipmunk_obj_shape_t shape;

                shape = ui_sprite_chipmunk_obj_shape_create_mamaged_from_body(body);
                if (shape == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: create body '%s': create shape fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
                    return -1;
                }
                fixture_data = shape->m_fixture_data;

                if (ui_sprite_chipmunk_load_fixture_shape(env, fixture_data, shape_cfg) != 0) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: create body '%s': shape shaoe fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
                    return -1;
                }

                if (ui_sprite_chipmunk_load_fixture_data(env, fixture_data, shape_cfg) != 0) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk obj load: create body '%s': load shape detail fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
                    return -1;
                }
            }

            if (ui_sprite_chipmunk_load_body_attrs(body->m_obj->m_env, &body->m_body_attrs, body_cfg) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk obj load: load body %s detail fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
                return -1;
            }
        }
    }

    return 0;
}

#ifdef __cplusplus
}
#endif

