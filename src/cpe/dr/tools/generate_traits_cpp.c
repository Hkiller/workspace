#include <assert.h>
#include <ctype.h>
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "generate_ops.h"

int cpe_dr_generate_traits_cpp(write_stream_t stream, const char * arg_name, cpe_dr_generate_ctx_t ctx) {
    struct dr_metalib_source_it source_it;
    dr_metalib_source_t source;
    struct dr_metalib_source_element_it element_it;
    dr_metalib_source_element_t element;
    struct mem_buffer buffer;

    int rv;

    assert(stream);
    assert(ctx->m_metalib);

    rv = 0;

    mem_buffer_init(&buffer, 0);

    stream_printf(stream, "#include \"cpe/pal/pal_external.h\"\n");
    stream_printf(stream, "#include \"cpepp/dr/MetaLib.hpp\"\n");
    stream_printf(stream, "#include \"cpepp/dr/Meta.hpp\"\n");

    dr_metalib_builder_sources(&source_it, ctx->m_builder);
    while((source = dr_metalib_source_next(&source_it))) {
        const char * file_name;

        if (dr_metalib_source_from(source) != dr_metalib_source_from_user) continue;

        file_name = dr_metalib_source_file(source);
        if (file_name == NULL) continue;

        mem_buffer_clear_data(&buffer);
        file_name_append_base(&buffer, file_name);
        mem_buffer_strcat(&buffer, ".h");

        stream_printf(stream, "#include \"%s\"\n", (char *)mem_buffer_make_continuous(&buffer, 0));
    }

    stream_printf(stream, "\nextern \"C\" { extern unsigned char %s[]; }", arg_name);
    
    stream_printf(stream, "\nnamespace Cpe { namespace Dr {\n");

    dr_metalib_builder_sources(&source_it, ctx->m_builder);
    while((source = dr_metalib_source_next(&source_it))) {
        const char * file_name;

        if (dr_metalib_source_from(source) != dr_metalib_source_from_user) continue;

        file_name = dr_metalib_source_file(source);
        if (file_name == NULL) continue;

        dr_metalib_source_elements(&element_it, source);
        while((element = dr_metalib_source_element_next(&element_it))) {
            LPDRMETA meta;
            const char * meta_name;

            if (dr_metalib_source_element_type(element) != dr_metalib_source_element_type_meta) continue;

            meta = dr_lib_find_meta_by_name(ctx->m_metalib, dr_metalib_source_element_name(element));
            if (meta == NULL) continue;
        
            meta_name = dr_meta_name(meta);
            stream_printf(stream, "\n");
            stream_printf(stream, "Meta const & MetaTraits<");
            stream_toupper(stream, meta_name);
            stream_printf(stream, ">::META = MetaLib::_cast((LPDRMETALIB)%s).meta(\"%s\");\n", arg_name, meta_name);

            stream_printf(stream, "const char * const MetaTraits<");
            stream_toupper(stream, meta_name);
            stream_printf(stream, ">::NAME = \"%s\";\n", meta_name);
        }
    }

    stream_printf(stream, "\n}}\n");
    stream_printf(stream, "\n");

    mem_buffer_clear(&buffer);

    return rv;
}

