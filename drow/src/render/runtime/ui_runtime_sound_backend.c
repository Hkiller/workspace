#include "cpe/utils/string_utils.h"
#include "ui_runtime_sound_backend_i.h"
#include "ui_runtime_sound_chanel_i.h"

ui_runtime_sound_backend_t
ui_runtime_sound_backend_create(
    ui_runtime_module_t module, const char * name,
    void * ctx,
    ui_runtime_sound_backend_res_install_t res_install,
    ui_runtime_sound_backend_res_uninstall_t res_uninstall,
    uint16_t chanel_capacity,
    ui_runtime_sound_backend_chanel_init_t chanel_init,
    ui_runtime_sound_backend_chanel_fini_t chanel_fini,
    ui_runtime_sound_backend_chanel_play_t chanel_play,
    ui_runtime_sound_backend_chanel_stop_t chanel_stop,
    ui_runtime_sound_backend_chanel_pause_t chanel_pause,
    ui_runtime_sound_backend_chanel_resume_t chanel_resume,
    ui_runtime_sound_backend_chanel_get_state_t chanel_get_state,
    ui_runtime_sound_backend_chanel_set_volumn_t chanel_set_volumn)
{
    ui_runtime_sound_backend_t backend;

    backend = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_sound_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_sound_backend_create: alloc fail!");
        return NULL;
    }

    cpe_str_dup(backend->m_name, sizeof(backend->m_name), name);
    backend->m_module = module;
    backend->m_ctx = ctx;
    backend->m_res_install = res_install;
    backend->m_res_uninstall = res_uninstall;
    backend->m_chanel_capacity = chanel_capacity;
    backend->m_chanel_init = chanel_init;
    backend->m_chanel_fini = chanel_fini;
    backend->m_chanel_pause = chanel_pause;
    backend->m_chanel_resume = chanel_resume;
    backend->m_chanel_get_state = chanel_get_state;
    backend->m_chanel_play = chanel_play;
    backend->m_chanel_stop = chanel_stop;
    backend->m_chanel_set_volumn = chanel_set_volumn;

    TAILQ_INIT(&backend->m_chanels);
    
    TAILQ_INSERT_TAIL(&module->m_sound_backends, backend, m_next);
    return backend;
}
    
void ui_runtime_sound_backend_free(ui_runtime_sound_backend_t backend) {
    while(!TAILQ_EMPTY(&backend->m_chanels)) {
        ui_runtime_sound_chanel_free(TAILQ_FIRST(&backend->m_chanels));
    }
    
    TAILQ_REMOVE(&backend->m_module->m_sound_backends, backend, m_next);
    mem_free(backend->m_module->m_alloc, backend);
}

ui_runtime_sound_backend_t
ui_runtime_sound_backend_find_by_name(ui_runtime_module_t module, const char * name) {
    ui_runtime_sound_backend_t backend;

    TAILQ_FOREACH(backend, &module->m_sound_backends, m_next) {
        if (strcmp(backend->m_name, name) == 0) return backend;
    }
    
    return NULL;
}

ui_runtime_sound_backend_t
ui_runtime_sound_backend_find(ui_runtime_module_t module, ui_runtime_sound_type_t sound_type) {
    ui_runtime_sound_backend_t backend;

    TAILQ_FOREACH(backend, &module->m_sound_backends, m_next) {
        return backend;
    }
    
    return NULL;
}

