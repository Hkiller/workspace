#include <assert.h>
#include "cpe/utils/stream.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"

static void dr_lib_print_meta(write_stream_t stream, LPDRMETA meta, int ident) {
    int i;

    stream_putc(stream, '\n');
    stream_putc_count(stream, ' ', ident);
    
    stream_printf(stream, "%s[%d]:", dr_meta_name(meta), dr_meta_id(meta));

    for(i = 0; i < dr_meta_entry_num(meta); ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(meta, i);

        stream_putc(stream, '\n');
        stream_putc_count(stream, ' ', ident + 4);

        stream_printf(
            stream, "%s: type=%s, start=%d, size=%d",
            dr_entry_name(entry), dr_entry_type_name(entry), dr_entry_data_start_pos(entry, 0), dr_entry_size(entry));
        if (dr_entry_array_count(entry) != 1) {
            stream_printf(stream, ", array=%d", dr_entry_array_count(entry));
        }
    }
}

void dr_lib_print(write_stream_t stream, LPDRMETALIB metaLib, int ident) {
    int i;
    int meta_count;

    stream_putc_count(stream, ' ', ident);

    if (metaLib == NULL) {
        stream_printf(stream, "invalid METALIB!!!");
        return;
    }

    stream_printf(
        stream, "meta-lib: name=%s, version=%d, build-version=%d, size=%d", 
        dr_lib_name(metaLib), dr_lib_version(metaLib), dr_lib_build_version(metaLib), dr_lib_size(metaLib));

    meta_count = dr_lib_meta_num(metaLib);
    for(i = 0; i < meta_count; ++i) {
        dr_lib_print_meta(stream, dr_lib_meta_at(metaLib, i), ident + 4);
    }

    stream_putc(stream, '\n');
}

const char * dr_lib_dump(mem_buffer_t buffer, LPDRMETALIB metaLib, int ident) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);
    
    dr_lib_print((write_stream_t)&stream, metaLib, ident);
    stream_putc((write_stream_t)&stream, 0);
    
    return mem_buffer_make_continuous(buffer, 0);
}
