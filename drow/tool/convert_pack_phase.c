#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "convert_pack_phase.h"

convert_pack_phase_t
convert_pack_phase_create(convert_ctx_t ctx, const char * phase_name) {
    convert_pack_phase_t phase;
    char buf[128];

    phase = mem_alloc(ctx->m_alloc, sizeof(struct convert_pack_phase));
    if (phase == NULL) {
        CPE_ERROR(ctx->m_em, "convert_pack_phase_check_create: alloc fail!");
        return NULL;
    }

    phase->m_ctx = ctx;
    cpe_str_dup(phase->m_name, sizeof(phase->m_name), phase_name);

    snprintf(buf, sizeof(buf), "phase/%s", phase_name);
    phase->m_package = plugin_package_package_create(ctx->m_package_module, buf, plugin_package_package_loaded);
    if (phase->m_package == NULL) {
        CPE_ERROR(ctx->m_em, "convert_pack_phase_check_create: create packer fail!");
        mem_free(ctx->m_alloc, phase);
        return NULL;
    }

    /* if (plugin_package_package_add_base_package(phase->m_package, ctx->m_global_package) != 0) { */
    /*     CPE_ERROR(ctx->m_em, "convert_pack_phase_check_create: add base backage fail!"); */
    /*     plugin_package_package_free(phase->m_package); */
    /*     mem_free(ctx->m_alloc, phase); */
    /*     return NULL; */
    /* } */
    
    TAILQ_INSERT_TAIL(&ctx->m_pack_phases, phase, m_next);

    return phase;
}

void convert_pack_phase_free(convert_pack_phase_t phase) {
    plugin_package_package_free(phase->m_package);
    TAILQ_REMOVE(&phase->m_ctx->m_pack_phases, phase, m_next);
    mem_free(phase->m_ctx->m_alloc, phase);
}

convert_pack_phase_t convert_pack_phase_find(convert_ctx_t ctx, const char * phase_name) {
    convert_pack_phase_t phase;

    TAILQ_FOREACH(phase, &ctx->m_pack_phases, m_next) {
        if (strcmp(phase->m_name, phase_name) == 0) return phase;
    }

    return NULL;
}

convert_pack_phase_t
convert_pack_phase_check_create(convert_ctx_t ctx, const char * phase_name) {
    convert_pack_phase_t phase;

    phase = convert_pack_phase_find(ctx, phase_name);

    return phase ? phase : convert_pack_phase_create(ctx, phase_name);
}

