#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_fsm_ins_action_i.h"

uint8_t ui_sprite_fsm_action_calc_bool_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, uint8_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    uint8_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_bool_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

int8_t ui_sprite_fsm_action_calc_int8_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, int8_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int8_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_int8_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

uint8_t ui_sprite_fsm_action_calc_uint8_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, uint8_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    uint8_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_uint8_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

int16_t ui_sprite_fsm_action_calc_int16_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, int16_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int16_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_int16_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

uint16_t ui_sprite_fsm_action_calc_uint16_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, uint16_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    uint16_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_uint16_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

int32_t ui_sprite_fsm_action_calc_int32_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, int32_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int32_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_int32_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

uint32_t ui_sprite_fsm_action_calc_uint32_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, uint32_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    uint32_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_uint32_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

int64_t ui_sprite_fsm_action_calc_int64_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, int64_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int64_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_int64_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

uint64_t ui_sprite_fsm_action_calc_uint64_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, uint64_t dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    uint64_t rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_uint64_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

float ui_sprite_fsm_action_calc_float_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, float dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    float rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_float_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

double ui_sprite_fsm_action_calc_double_with_dft(const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, double dft) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    double rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_double_with_dft(def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

const char * ui_sprite_fsm_action_calc_str_with_dft(
    mem_buffer_t buffer, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, const char * dft)
{
    struct ui_sprite_fsm_addition_source_ctx ctx;
    const char * rv;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    rv = ui_sprite_entity_calc_str_with_dft(buffer, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, dft);

    *ctx.m_last_source = NULL;

    return rv;
}

int ui_sprite_fsm_action_try_calc_bool(uint8_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_bool(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_int8(int8_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_int8(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_uint8(uint8_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_uint8(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_int16(int16_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_int16(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_uint16(uint16_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_uint16(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_int32(int32_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_int32(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_uint32(uint32_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_uint32(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_int64(int64_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_int64(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_uint64(uint64_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_uint64(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_float(float * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_float(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_try_calc_double(double * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_double(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

const char * ui_sprite_fsm_action_try_calc_str(
    mem_buffer_t buffer, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    struct ui_sprite_fsm_addition_source_ctx ctx;
    const char * r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_str(buffer, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

char * ui_sprite_fsm_action_try_calc_str_dup(
    mem_allocrator_t alloc, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    struct ui_sprite_fsm_addition_source_ctx ctx;
    char * r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_try_calc_str_dup(alloc, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_check_calc_bool(
    uint8_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_bool(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_bool(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_int8(
    int8_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_int8(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_int8(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_uint8(
    uint8_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_uint8(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_uint8(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_int16(
    int16_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_int16(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_int16(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_uint16(
    uint16_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_uint16(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_uint16(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_int32(
    int32_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_int32(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_int32(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_uint32(
    uint32_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_uint32(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_uint32(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_int64(
    int64_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_int64(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_int64(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_uint64(
    uint64_t * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_uint64(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_uint64(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_float(
    float * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_float(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_float(result, def);
    }
}

int ui_sprite_fsm_action_check_calc_double(
    double * result, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_double(result, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_to_double(result, def);
    }
}

const char * ui_sprite_fsm_action_check_calc_str(
    mem_buffer_t buffer, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_str(buffer, def + 1, fsm_action, data_source, em);
    }
    else {
        return def;
    }
}

char * ui_sprite_fsm_action_check_calc_str_dup(
    mem_allocrator_t alloc, const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_fsm_action_try_calc_str_dup(alloc, def + 1, fsm_action, data_source, em);
    }
    else {
        return cpe_str_mem_dup(alloc, def);
    }
}

int ui_sprite_fsm_action_check_calc_entity_id(
    uint32_t * result,
    const char * def, ui_sprite_fsm_action_t fsm_action, dr_data_source_t data_source, error_monitor_t em)
{
    struct ui_sprite_fsm_addition_source_ctx ctx;
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    r = ui_sprite_entity_check_calc_entity_id(result, def, ui_sprite_fsm_action_to_entity(fsm_action), data_source, em);

    *ctx.m_last_source = NULL;

    return r;
}
