#include "cpe/cfg/cfg.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_group.h"
#include "ui/app_manip/ui_app_manip_src.h"
#include "convert_pack_phase.h"

static int convert_pack_phase_audios(convert_ctx_t ctx, convert_pack_phase_t phase, cfg_t phase_cfg);

int convert_load_from_phases(convert_ctx_t ctx) {
    struct cfg_it phase_it;
    cfg_t phase_cfg;
    int rv = 0;
    
    cfg_it_init(&phase_it, cfg_find_cfg(ctx->m_runing, "ui.phases"));

    while((phase_cfg = cfg_it_next(&phase_it))) {
        const char * phase_name = cfg_name(phase_cfg);
        convert_pack_phase_t phase;
        struct cfg_it child_it;
        cfg_t child_cfg;
        
        phase = convert_pack_phase_check_create(ctx, phase_name);
        if (phase == NULL) {
            CPE_ERROR(ctx->m_em, "convert_pack_phase_from_phases: phase %s create fail", phase_name);
            rv = -1;
            continue;
        }

        cfg_it_init(&child_it, cfg_find_cfg(phase_cfg, "packages"));
        while((child_cfg = cfg_it_next(&child_it))) {
            const char * base_package_name;
            plugin_package_package_t base_package;
            
            base_package_name = cfg_as_string(child_cfg, NULL);
            if (base_package_name == NULL) {
                CPE_ERROR(ctx->m_em, "convert_pack_phase_from_phases: phase %s load base package format error", phase_name);
                rv = -1;
                continue;
            }

            base_package = plugin_package_package_find(ctx->m_package_module, base_package_name);
            if (base_package == NULL) { 
                base_package = plugin_package_package_create(ctx->m_package_module, base_package_name, plugin_package_package_loaded);
                if (base_package == NULL) {
                    CPE_ERROR(ctx->m_em, "convert_pack_phase_from_phases: phase %s create base package %s fail", phase_name, base_package_name);
                    rv = -1;
                    continue;
                }
            }

            if (plugin_package_package_add_base_package(phase->m_package, base_package) != 0) {
                CPE_ERROR(ctx->m_em, "convert_pack_phase_from_phases: phase %s add base package %s fail", phase_name, base_package_name);
                rv = -1;
                continue;
            }
        }
        
        if (convert_pack_phase_audios(ctx, phase, phase_cfg) != 0) rv = -1;

        cfg_it_init(&child_it, phase_cfg);
        while((child_cfg = cfg_it_next(&child_it))) {
            const char * name = cfg_name(child_cfg);
            plugin_package_package_t package = phase->m_package;
            
            if (strcmp(name, "runing-fsm") == 0 || cpe_str_start_with(name, "runing-fsm-")) {
                const char * state_group_name = cpe_str_start_with(name, "runing-fsm-") ? name + strlen("runing-fsm-") : "";
                struct cfg_it state_package_group_it;
                cfg_t state_package_group_cfg;
                cfg_t state_package_cfg = NULL;
                char state_package_name[64];

                snprintf(state_package_name, sizeof(state_package_name), "%s-%s", phase_name, state_group_name);
                
                cfg_it_init(&state_package_group_it, cfg_find_cfg(ctx->m_package_def, "package-rules"));
                while((state_package_group_cfg = cfg_it_next(&state_package_group_it))) {
                    struct cfg_it state_package_it;
                    
                    state_package_group_cfg = cfg_child_only(state_package_group_cfg);

                    if (strcmp(cfg_name(state_package_group_cfg), "phase") != 0) continue;

                    cfg_it_init(&state_package_it, state_package_group_cfg);
                    while((state_package_cfg = cfg_it_next(&state_package_it))) {
                        if (strcmp(cfg_get_string(state_package_cfg, "name", ""), state_package_name) == 0) break;
                    }
                }

                if (state_package_cfg) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "phase/%s", state_package_name);
                    
                    package = plugin_package_package_find(ctx->m_package_module, buf);
                    if (package == NULL) {
                        package = plugin_package_package_create(ctx->m_package_module, buf, plugin_package_package_loaded);
                        if (package == NULL) {
                            CPE_ERROR(
                                ctx->m_em, "convert_pack_phase_from_phases: phase %s state %s create package fail",
                                phase_name, cfg_name(state_package_cfg));
                            continue;
                        }
                    }
                }
                
                if (ui_app_manip_collect_src_from_fsm(
                        plugin_package_package_srcs(package),
                        plugin_package_package_resources(package),
                        child_cfg, ctx->m_em)
                    != 0)
                {
                    CPE_ERROR(ctx->m_em, "convert_pack_phase_from_phases: collect src from fsm %s error", name);
                    rv = -1;
                }
            }
        }
    }
    
    return rv;
}

static int convert_pack_phase_audios(convert_ctx_t ctx, convert_pack_phase_t phase, cfg_t phase_cfg) {
    struct cfg_it audio_it;
    cfg_t audio_cfg;
    int rv = 0;
    
    cfg_it_init(&audio_it, cfg_find_cfg(phase_cfg, "play-bgm"));
    while((audio_cfg = cfg_it_next(&audio_it))) {
        const char * path = cfg_as_string(audio_cfg, NULL);

        if (path == NULL) {
            CPE_ERROR(ctx->m_em, "convert_pack_phase_from_phases: audion format error");
            rv = -1;
            continue;
        }

        if (ui_cache_group_add_res_by_path(plugin_package_package_resources(phase->m_package), path) != 0) {
            rv = -1;
        }
    }

    return rv;
}
