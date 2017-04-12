#include "ui_cache_sound_buf_i.h"

ui_cache_sound_buf_t ui_cache_sound_buf_create(ui_cache_manager_t mgr) {
    ui_cache_sound_buf_t buf;
    
    buf = mem_calloc(mgr->m_alloc, sizeof(struct ui_cache_sound_buf));
    if (buf == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create sound buf fail!", ui_cache_manager_name(mgr));
        return NULL;
    }
    
    buf->m_mgr = mgr;
    buf->m_format = ui_cache_sound_format_unknown;

    return buf;
}

void ui_cache_sound_buf_free(ui_cache_sound_buf_t buf) {
    ui_cache_manager_t mgr = buf->m_mgr;

    if (buf->m_data) mem_free(mgr->m_alloc, buf->m_data);
    buf->m_data = NULL;

    mem_free(mgr->m_alloc, buf);
}

uint32_t ui_cache_sound_buf_freq(ui_cache_sound_buf_t buf) {
    return buf->m_freq;
}

uint32_t ui_cache_sound_buf_bytes_per_sec(ui_cache_sound_buf_t buf) {
    return buf->m_bytes_per_sec;
}

uint8_t ui_cache_sound_buf_channel(ui_cache_sound_buf_t buf) {
    return buf->m_channel;
}

ui_cache_sound_data_format_t ui_cache_sound_buf_data_format(ui_cache_sound_buf_t buf) {
    return buf->m_data_format;
}

uint32_t ui_cache_sound_buf_bits_per_sample(ui_cache_sound_buf_t buf) {
    return buf->m_bits_per_sample;
}

uint32_t ui_cache_sound_buf_data_size(ui_cache_sound_buf_t buf) {
    return buf->m_data_size;
}

void * ui_cache_sound_buf_data(ui_cache_sound_buf_t buf) {
    return buf->m_data;
}
