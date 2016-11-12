#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_init.h"
#include "generate_ops.h"

int cpe_dr_generate_lib_c(write_stream_t stream, const char * arg_name, cpe_dr_generate_ctx_t ctx) {
    int rv;
    unsigned char * buf;
    size_t size;
    int first_line;

    assert(stream);
    assert(arg_name);
    assert(ctx->m_metalib);

    rv = 0;

    buf = (unsigned char *)ctx->m_metalib;
    size = dr_lib_size(ctx->m_metalib);

    first_line = 1;

    stream_printf(stream, "#include \"cpe/pal/pal_external.h\"\n EXPORT_DIRECTIVE\nchar %s[] = {", arg_name);

    while(size > 0) {
        size_t i;
        size_t line_size = size > 16 ? 16 : size;

        stream_printf(stream, "\n    ");

        if (first_line) {
            stream_printf(stream, "  ");
            first_line = 0;
        }
        else {
            stream_printf(stream, ", ");
        }

        for(i = 0; i < line_size; ++i, ++buf) {
            if (i > 0) { stream_printf(stream, ", "); }

            stream_printf(stream, "%s", "0x");
            stream_printf(stream, "%.2X", *buf);
        }

        size -= line_size;
    }

    stream_printf(stream, "\n};\n");

    return rv;
}

