#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "app_module_installer_i.h"
#include "app_internal_ops.h"

static void gd_app_module_installer_move_to_next(gd_app_module_installer_t installer);

gd_app_module_installer_t
gd_app_module_installer_create(gd_app_context_t app, cfg_t cfg) {
    gd_app_module_installer_t installer;

    installer = mem_alloc(app->m_alloc, sizeof(struct gd_app_module_installer));
    if (installer == NULL) {
        CPE_ERROR(app->m_em, "gd_app_module_installer_create: alloc fail!");
        return NULL;
    }

    installer->m_app = app;
    installer->m_next = NULL;
    installer->m_stack_size = 1;
    installer->m_stack[0].m_cfg = cfg;
    installer->m_rv = 0;
    cfg_it_init(&installer->m_stack[0].m_childs_it, cfg);

    gd_app_module_installer_move_to_next(installer);
    
    return installer; 
}

void gd_app_module_installer_free(gd_app_module_installer_t installer) {
    mem_free(installer->m_app->m_alloc, installer);
}

static uint8_t gd_app_module_installer_need_process_by_tag(gd_app_module_installer_t installer, cfg_t check_cfg) {
    cfg_t tag_cfg;
    struct cfg_it tag_cfg_it;
    
    cfg_it_init(&tag_cfg_it, cfg_find_cfg(check_cfg, "tags"));
    while((tag_cfg = cfg_it_next(&tag_cfg_it))) {
        const char * tag = cfg_as_string(tag_cfg, NULL);
        if (tag == NULL) continue;

        if (tag[0] == '-') {
            if (gd_app_have_tag(installer->m_app, tag + 1)) {
                return 0;
            }
        }
        else {
            if (!gd_app_have_tag(installer->m_app, tag)) {
                return 0;
            }
        }
    }

    return 1;
}

static void gd_app_module_installer_move_to_next(gd_app_module_installer_t installer) {
    installer->m_next = NULL;

    while(installer->m_next == NULL && installer->m_stack_size > 0) {
        struct gd_app_module_installer_stack_node * node;
        cfg_t module_cfg;

        node = &installer->m_stack[installer->m_stack_size - 1];

    CHECK_AGAIN:        
        while((module_cfg = cfg_it_next(&node->m_childs_it))) {
            const char * buf;
            
            if (!gd_app_module_installer_need_process_by_tag(installer, module_cfg)) continue;

            buf = cfg_get_string(module_cfg, "name", NULL);
            if (buf) break;

            buf = cfg_get_string(module_cfg, "include", NULL);
            if (buf) {
                cfg_t include_cfg = cfg_find_cfg(cfg_find_cfg(cfg_parent(installer->m_stack[0].m_cfg), buf), "load");
                if (include_cfg == NULL) {
                    APP_CTX_ERROR(
                        installer->m_app, "app: load module [%s]: config type error!",
                        cfg_path(gd_app_tmp_buffer(installer->m_app), module_cfg, 0));
                    installer->m_rv = -1;
                    return;
                }

                if (installer->m_stack_size >= CPE_ARRAY_SIZE(installer->m_stack)) {
                    APP_CTX_ERROR(
                        installer->m_app, "app: load module [%s]: stack overflow!",
                        cfg_path(gd_app_tmp_buffer(installer->m_app), module_cfg, 0));
                    installer->m_rv = -1;
                    return;
                }

                node = &installer->m_stack[installer->m_stack_size++];
                node->m_cfg = include_cfg;
                cfg_it_init(&node->m_childs_it, include_cfg);
                goto CHECK_AGAIN;
            }
        }
        
        if (module_cfg == NULL) {
            installer->m_stack_size--;
            continue;
        }

        installer->m_next = module_cfg;;
    }

    return;
}

uint8_t gd_app_module_installer_is_done(gd_app_module_installer_t installer) {
    return installer->m_stack_size == 0 ? 1 : 0;
}

int gd_app_module_installer_install_one(gd_app_module_installer_t installer) {
    int rv;
    
    if (installer->m_next) {
        rv = gd_app_module_create(installer->m_app, cfg_get_string(installer->m_next, "name", NULL), installer->m_next);
    }
    else {
        rv = installer->m_rv;
    }

    installer->m_rv = 0;    
    gd_app_module_installer_move_to_next(installer);
    
    return rv;
}
