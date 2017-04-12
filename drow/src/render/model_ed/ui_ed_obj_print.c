#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "render/model/ui_data_action.h"
#include "render/model/ui_data_layout.h"
#include "ui_ed_obj_i.h"
#include "ui_ed_src_i.h"

void ui_ed_obj_path_print(write_stream_t s, ui_ed_obj_t obj) {
    if(obj->m_parent) {
        ui_ed_obj_path_print(s, obj->m_parent);
        stream_printf(s, ".");
    }

    switch(obj->m_meta->m_type) {
    case ui_ed_obj_type_src:
        stream_printf(s, "src(%u)", ((UI_ED_SRC const *)obj->m_data)->src_id);
        break;
    case ui_ed_obj_type_img_block:
        stream_printf(s, "img_block(%u)", ((UI_IMG_BLOCK const *)obj->m_data)->id);
        break;
    case ui_ed_obj_type_frame:
        stream_printf(s, "frame(%u)", ((UI_FRAME const *)obj->m_data)->id);
        break;
    case ui_ed_obj_type_frame_img:
        stream_printf(s, "frame_img(%u-%u)", ((UI_IMG_REF const *)obj->m_data)->module_id, ((UI_IMG_REF const *)obj->m_data)->img_block_id);
        break;
    case ui_ed_obj_type_actor:
    case ui_ed_obj_type_actor_layer:
    case ui_ed_obj_type_actor_frame:
        break;
    default:
        stream_printf(s, "type(%d)", obj->m_meta->m_type);
        break;
    }
}

const char * ui_ed_obj_path_dump(mem_buffer_t buffer, ui_ed_obj_t obj) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    ui_ed_obj_path_print((write_stream_t)&stream, obj);

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

const char * ui_ed_obj_full_path_dump(mem_buffer_t buffer, ui_ed_obj_t obj) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    ui_ed_src_path_print((write_stream_t)&stream, obj->m_src);
    stream_printf((write_stream_t)&stream, ": ");

    ui_ed_obj_path_print((write_stream_t)&stream, obj);

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

const char * ui_ed_obj_dump_with_full_path(mem_buffer_t buffer, ui_ed_obj_t obj) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    ui_ed_src_path_print((write_stream_t)&stream, obj->m_src);
    stream_printf((write_stream_t)&stream, ": ");

    ui_ed_obj_path_print((write_stream_t)&stream, obj);
    stream_printf((write_stream_t)&stream, ": ");

    dr_json_print((write_stream_t)&stream, obj->m_data, obj->m_data_capacity, obj->m_meta->m_data_meta, DR_JSON_PRINT_MINIMIZE, NULL);
    
    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

