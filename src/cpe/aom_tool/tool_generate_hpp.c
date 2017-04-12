#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "tool_env.h"

enum aom_tool_do_generate_hpp_print_ns_policy {
    aom_tool_do_generate_hpp_print_ns_normal,
    aom_tool_do_generate_hpp_print_ns_upcase,
    aom_tool_do_generate_hpp_print_ns_none
};

static void aom_tool_do_generate_hpp_print_ns(
    write_stream_t stream, const char * namespace, 
    enum aom_tool_do_generate_hpp_print_ns_policy policy,
    const char * prefix, const char * postfix)
{
    while(*namespace) {
        char * ns_end;
        size_t len;

        stream_write(stream, prefix, strlen(prefix));

        ns_end = strstr(namespace, "::");

        len = ns_end ? ns_end - namespace : strlen(namespace);

        switch(policy) {
        case aom_tool_do_generate_hpp_print_ns_normal:
            stream_write(stream, namespace, len);
            break;
        case aom_tool_do_generate_hpp_print_ns_upcase:
            stream_toupper_len(stream, namespace, len);
            break;
        case aom_tool_do_generate_hpp_print_ns_none:
            break;
        }

        stream_write(stream, postfix, strlen(postfix));

        if (ns_end) {
            namespace = ns_end + 2;
        }
        else {
            namespace += strlen(namespace);
        }
    }
}

static void aom_tool_do_generate_hpp_read_ops_normal(write_stream_t stream, LPDRMETAENTRY entry, error_monitor_t em) {
    stream_printf(stream, "    ");
    stream_toupper(stream, dr_meta_name(dr_entry_ref_meta(entry)));
    stream_printf(stream, " const & %s(void) const { return m_data->%s; } \n", dr_entry_name(entry), dr_entry_name(entry));
}

static void aom_tool_do_generate_hpp_read_ops_list(write_stream_t stream, LPDRMETAENTRY entry, error_monitor_t em) {
    char typename[128];
    const char * entry_name = dr_entry_name(entry);
    LPDRMETAENTRY ref_entry = dr_entry_array_refer_entry(entry);
    const char * count_entry_name = dr_entry_name(ref_entry);

    cpe_str_dup(typename, sizeof(typename), dr_meta_name(dr_entry_ref_meta(entry)));
    cpe_str_toupper(typename);

    stream_printf(
        stream, "    uint16_t %sCount(void) const { return m_data->%s; }\n",
        entry_name, dr_entry_name(ref_entry));

    stream_printf(
        stream, "    uint16_t %sCapacity(void) const { return sizeof(m_data->%s) / sizeof(m_data->%s[0]); }\n"
        , entry_name, entry_name, entry_name);

    stream_printf(
        stream, "    %s const & %sAt(uint16_t pos) const { return m_data->%s[pos]; } \n",
        typename, entry_name, entry_name);

    stream_printf(stream, "    %s const * %sLSearch(%s const & key, int (*cmp)(%s const & l, %s const & r)) const {\n", typename, entry_name, typename, typename, typename);
    stream_printf(stream, "        for(uint16_t i = 0; i < m_data->%s; ++i) {\n", count_entry_name);
    stream_printf(stream, "            if (cmp(m_data->%s[i], key) == 0) return &m_data->%s[i];\n", entry_name, entry_name);
    stream_printf(stream, "        }\n");
    stream_printf(stream, "        return NULL;\n");
    stream_printf(stream, "    }\n");

    stream_printf(stream, "    template<typename EqualT, typename KeyT>\n");
    stream_printf(stream, "    %s const * %sLSearch(%s const & key, EqualT cmp = EqualT()) const {\n", typename, entry_name, typename);
    stream_printf(stream, "        for(uint16_t i = 0; i < m_data->%s; ++i) {\n", count_entry_name);
    stream_printf(stream, "            if (cmp(m_data->%s[i], key)) return &m_data->%s[i];\n", entry_name, entry_name);
    stream_printf(stream, "        }\n");
    stream_printf(stream, "        return NULL;\n");
    stream_printf(stream, "    }\n");

    stream_printf(stream, "    template<typename EqualT, typename KeyT>\n");
    stream_printf(stream, "    int16_t %sLSearchPos(KeyT const & key, EqualT cmp = EqualT()) const {\n", entry_name, typename);
    stream_printf(stream, "        for(uint16_t i = 0; i < m_data->%s; ++i) {\n", count_entry_name);
    stream_printf(stream, "            if (cmp(m_data->%s[i], key)) return (int16_t)i;\n", entry_name);
    stream_printf(stream, "        }\n");
    stream_printf(stream, "        return -1;\n");
    stream_printf(stream, "    }\n");
}

static void aom_tool_do_generate_hpp_read_ops_ba(write_stream_t stream, LPDRMETAENTRY entry, error_monitor_t em) {
    const char * entry_name = dr_entry_name(entry);

    stream_printf(stream, "    enum { ");
    stream_toupper(stream, entry_name);
    stream_printf(stream, "_BA_BYTES = %d };\n", (int)dr_entry_element_size(entry));

    stream_printf(stream, "    enum { ");
    stream_toupper(stream, entry_name);
    stream_printf(stream, "_BA_BITS = %d };\n", (int)(dr_entry_element_size(entry) * 8));

    stream_printf(
        stream, "    uint16_t %sCount(void) const { return cpe_ba_count((cpe_ba_t)m_data->%s, %d); }\n",
        entry_name, entry_name, (int)(dr_entry_element_size(entry) * 8));
    
    stream_printf(
        stream, "    cpe_ba_value_t %sGet(uint32_t pos) const { return cpe_ba_get((cpe_ba_t)m_data->%s, pos); }\n",
        entry_name, entry_name);

    stream_printf(
        stream,
        "    void  %sBinaryGet(void * data, uint32_t capacity) const {\n"
        "        memcpy(data, m_data->%s, capacity > sizeof(m_data->%s) ? sizeof(m_data->%s) : capacity);\n"
        "    }\n",
        entry_name, entry_name, entry_name, entry_name);
}

static void aom_tool_do_generate_hpp_read_ops(write_stream_t stream, struct aom_tool_env * env) {
    size_t i;

    for(i = 0; i < dr_meta_entry_num(env->m_meta); ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(env->m_meta, i);
        int entry_type = dr_entry_type(entry);
        int element_count = dr_entry_array_count(entry);

        if (entry_type == CPE_DR_TYPE_STRUCT) {
            stream_printf(stream, "\n");

            if (element_count == 1) {
                aom_tool_do_generate_hpp_read_ops_normal(stream, entry, env->m_em);
            }
            else {
                aom_tool_do_generate_hpp_read_ops_list(stream, entry, env->m_em);
            }
        }
        else if (entry_type == CPE_DR_TYPE_UINT8
                 && element_count > 1
                 && dr_entry_array_refer_entry(entry) == NULL)
        {
            stream_printf(stream, "\n");

            aom_tool_do_generate_hpp_read_ops_ba(stream, entry, env->m_em);
        }
    }
}

static void aom_tool_do_generate_hpp_write_ops_normal(write_stream_t stream, LPDRMETAENTRY entry, error_monitor_t em) {
    stream_printf(stream, "    ");
    stream_toupper(stream, dr_meta_name(dr_entry_ref_meta(entry)));
    stream_printf(stream, "  & _%s(void) { return m_data->%s; }\n", dr_entry_name(entry), dr_entry_name(entry));
}

static void aom_tool_do_generate_hpp_write_ops_list(write_stream_t stream, LPDRMETAENTRY entry, error_monitor_t em) {
    char typename[128];
    const char * entry_name = dr_entry_name(entry);
    LPDRMETAENTRY ref_entry = dr_entry_array_refer_entry(entry);
    const char * count_entry_name = dr_entry_name(ref_entry);

    cpe_str_dup(typename, sizeof(typename), dr_meta_name(dr_entry_ref_meta(entry)));
    cpe_str_toupper(typename);

    stream_printf(
        stream, "    %s & _%sAt(uint16_t pos) { return m_data->%s[pos]; } \n",
        typename, entry_name, entry_name);

    stream_printf(stream, "    void _%sAppend(%s const & data) {\n", entry_name, typename);
    stream_printf(stream, "        if(m_data->%s >= %sCapacity()) throw ::std::runtime_error(\"%s append overflow!\");\n", count_entry_name, entry_name, entry_name);
    stream_printf(stream, "        m_data->%s[m_data->%s++] = data;\n", entry_name, count_entry_name);
    stream_printf(stream, "    }\n");

    stream_printf(stream, "    void _%sInsert(uint16_t pos, %s const & data) {\n", entry_name, typename);
    stream_printf(stream, "        if(pos > (uint16_t)m_data->%s) throw ::std::runtime_error(\"%s insert overflow!\");\n", count_entry_name, entry_name);
    stream_printf(stream, "        if(pos < (uint16_t)m_data->%s) {\n", count_entry_name);
    stream_printf(stream, "            memmove(&m_data->%s[pos + 1], &m_data->%s[pos], sizeof(m_data->%s[0]) * ((uint16_t)m_data->%s - pos));\n", entry_name, entry_name, entry_name, count_entry_name);
    stream_printf(stream, "        }\n");
    stream_printf(stream, "        m_data->%s[pos] = data;\n", entry_name);
    stream_printf(stream, "        m_data->%s++;\n", count_entry_name);
    stream_printf(stream, "    }\n");

    stream_printf(stream, "    void _%sRemove(uint16_t pos) {\n", entry_name, typename);
    stream_printf(stream, "        if(pos >= (uint16_t)m_data->%s) throw ::std::runtime_error(\"%s insert overflow!\");\n", count_entry_name, entry_name);
    stream_printf(stream, "        if(pos + 1 < (uint16_t)m_data->%s) {\n", count_entry_name);
    stream_printf(stream, "            memmove(&m_data->%s[pos], &m_data->%s[pos + 1], sizeof(m_data->%s[0]) * ((uint16_t)m_data->%s - pos - 1));\n", entry_name, entry_name, entry_name, count_entry_name);
    stream_printf(stream, "        }\n");
    stream_printf(stream, "        m_data->%s--;\n", count_entry_name);
    stream_printf(stream, "    }\n");
    stream_printf(stream, "    void _%sClear(void) { m_data->%s = 0; } \n", entry_name, count_entry_name);

    stream_printf(stream, "    %s * _%sLSearch(%s const & key, int (*cmp)(%s const & l, %s const & r)) {\n", typename, entry_name, typename, typename, typename);
    stream_printf(stream, "        for(uint16_t i = 0; i < m_data->%s; ++i) {\n", count_entry_name);
    stream_printf(stream, "            if (cmp(m_data->%s[i], key) == 0) return &m_data->%s[i];\n", entry_name, entry_name);
    stream_printf(stream, "        }\n");
    stream_printf(stream, "        return NULL;\n");
    stream_printf(stream, "    }\n");

    stream_printf(stream, "    template<typename EqualT, typename KeyT>\n");
    stream_printf(stream, "    %s * _%sLSearch(KeyT const & key, EqualT cmp = EqualT()) {\n", typename, typename);
    stream_printf(stream, "        for(uint16_t i = 0; i < m_data->%s; ++i) {\n", count_entry_name);
    stream_printf(stream, "            if (cmp(m_data->%s[i], key)) return &m_data->%s[i];\n", entry_name, entry_name);
    stream_printf(stream, "        }\n");
    stream_printf(stream, "        return NULL;\n");
    stream_printf(stream, "    }\n");
}

static void aom_tool_do_generate_hpp_write_ops_ba(write_stream_t stream, LPDRMETAENTRY entry, error_monitor_t em) {
    const char * entry_name = dr_entry_name(entry);

    stream_printf(
        stream, "    void _%sSet(uint32_t pos, cpe_ba_value_t value) { cpe_ba_set(m_data->%s, pos, value); }\n",
        entry_name, entry_name);
}

static void aom_tool_do_generate_hpp_write_ops(write_stream_t stream, struct aom_tool_env * env) {
    size_t i;

    for(i = 0; i < dr_meta_entry_num(env->m_meta); ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(env->m_meta, i);
        int entry_type = dr_entry_type(entry);
        int element_count = dr_entry_array_count(entry);

        if (entry_type == CPE_DR_TYPE_STRUCT) {
            stream_printf(stream, "\n");

            if (element_count == 1) {
                aom_tool_do_generate_hpp_write_ops_normal(stream, entry, env->m_em);
            }
            else {
                aom_tool_do_generate_hpp_write_ops_list(stream, entry, env->m_em);
            }
        }
        else if (entry_type == CPE_DR_TYPE_UINT8
                 && element_count > 1
                 && dr_entry_array_refer_entry(entry) == NULL)
        {
            stream_printf(stream, "\n");

            aom_tool_do_generate_hpp_write_ops_ba(stream, entry, env->m_em);
        }
    }
}

static void aom_tool_do_generate_hpp(write_stream_t stream, struct aom_tool_env * env, const char * classname, const char * namespace) {
    assert(stream);
    assert(env);
    assert(env->m_meta);

    stream_printf(stream, "#ifndef POM_GRP_GENERATED_H");
    aom_tool_do_generate_hpp_print_ns(stream, namespace, aom_tool_do_generate_hpp_print_ns_upcase, "_", "");
    stream_printf(stream, "_");
    stream_toupper(stream, classname);
    stream_printf(stream, "_INCLEDED\n");
    stream_printf(stream, "#define POM_GRP_GENERATED_H");
    aom_tool_do_generate_hpp_print_ns(stream, namespace, aom_tool_do_generate_hpp_print_ns_upcase, "_", "");
    stream_printf(stream, "_");
    stream_toupper(stream, classname);
    stream_printf(stream, "_INCLEDED\n");
    stream_printf(stream, "#include <stdexcept>\n");
    stream_printf(stream, "#include \"cpe/utils/bitarry.h\"\n");
    stream_printf(stream, "\n");

    stream_printf(stream, "#ifdef _MSC_VER\n");
    stream_printf(stream, "# pragma warning(push)\n");
    stream_printf(stream, "# pragma warning(disable:4344)\n");
    stream_printf(stream, "#endif\n");
    stream_printf(stream, "\n");

    aom_tool_do_generate_hpp_print_ns(stream, namespace, aom_tool_do_generate_hpp_print_ns_normal, "namespace ", " {");
    stream_printf(stream, "\n");
    stream_printf(stream, "\n");

    stream_printf(stream, "class %s {\n", classname);
    stream_printf(stream, "public:\n");
    stream_printf(stream, "    %s(", classname);
    stream_toupper(stream, dr_meta_name(env->m_meta));
    stream_printf(stream, " * data)\n", classname);
    stream_printf(stream, "        : m_data(data)\n");
    stream_printf(stream, "    {\n");
    stream_printf(stream, "    }\n");

    stream_printf(stream, "\n");
    stream_printf(stream, "    ");
    stream_toupper(stream, dr_meta_name(env->m_meta));
    stream_printf(stream, " const * data(void) const { return m_data; }\n");

    aom_tool_do_generate_hpp_read_ops(stream, env);    
    stream_printf(stream, "\n");

    stream_printf(stream, "protected:\n");
    stream_printf(stream, "    ");
    stream_toupper(stream, dr_meta_name(env->m_meta));
    stream_printf(stream, " * _data(void) { return m_data; }\n");
    stream_printf(stream, "\n");
    aom_tool_do_generate_hpp_write_ops(stream, env);    
    stream_printf(stream, "\n");

    stream_printf(stream, "private:\n");
    stream_printf(stream, "    ");
    stream_toupper(stream, dr_meta_name(env->m_meta));
    stream_printf(stream, " * m_data;\n");

    stream_printf(stream, "};\n");
    stream_printf(stream, "\n");
    aom_tool_do_generate_hpp_print_ns(stream, namespace, aom_tool_do_generate_hpp_print_ns_none, "", "}");
    stream_printf(stream, "\n");
    stream_printf(stream, "\n");

    stream_printf(stream, "#ifdef _MSC_VER\n");
    stream_printf(stream, "# pragma warning(pop)\n");
    stream_printf(stream, "#endif\n");
    stream_printf(stream, "\n");

    stream_printf(stream, "#endif\n");
}

int aom_tool_generate_hpp(struct aom_tool_env * env, const char * filename, const char * classname, const char * namespace) {
    struct write_stream_file stream;
    FILE * fp;

    if (env->m_meta == NULL) {
        CPE_ERROR(env->m_em, "generate hpp: no aom-meta!");
        return -1;
    }

    fp = file_stream_open(filename, "w", env->m_em);
    if (fp == NULL) {
        CPE_ERROR(env->m_em, "open %s fro generate lib c fail!", filename);
        return -1;
    }

    write_stream_file_init(&stream, fp, env->m_em);

    aom_tool_do_generate_hpp((write_stream_t)&stream, env, classname, namespace);

    file_stream_close(fp, env->m_em);

    return 0;
}


