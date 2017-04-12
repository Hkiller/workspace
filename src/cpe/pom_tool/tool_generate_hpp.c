#include <assert.h>
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "tool_env.h"

enum pom_tool_do_generate_hpp_print_ns_policy {
    pom_tool_do_generate_hpp_print_ns_normal,
    pom_tool_do_generate_hpp_print_ns_upcase,
    pom_tool_do_generate_hpp_print_ns_none
};

static cfg_t pom_tool_do_generate_hpp_get_entry_cfg(struct pom_tool_env * env, const char * entryName) {
    cfg_t root_cfg;
    cfg_t cfg;
    struct cfg_it it;

    if (env->m_pom_cfg == NULL) return NULL;

    root_cfg = cfg_child_only(env->m_pom_cfg);
    if (root_cfg == NULL) return NULL;

    cfg_it_init(&it, cfg_find_cfg(root_cfg, "attributes"));
    while((cfg = cfg_it_next(&it))) {
        cfg = cfg_child_only(cfg);
        if (strcmp(cfg_name(cfg), entryName) == 0) return cfg;
    }

    return NULL;
}

static void pom_tool_do_generate_hpp_print_ns(
    write_stream_t stream, const char * namespace, 
    enum pom_tool_do_generate_hpp_print_ns_policy policy,
    const char * prefix, const char * postfix)
{
    while(*namespace) {
        char * ns_end;
        size_t len;

        stream_write(stream, prefix, strlen(prefix));

        ns_end = strstr(namespace, "::");

        len = ns_end ? ns_end - namespace : strlen(namespace);

        switch(policy) {
        case pom_tool_do_generate_hpp_print_ns_normal:
            stream_write(stream, namespace, len);
            break;
        case pom_tool_do_generate_hpp_print_ns_upcase:
            stream_toupper_len(stream, namespace, len);
            break;
        case pom_tool_do_generate_hpp_print_ns_none:
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

static void pom_tool_do_generate_hpp_read_ops_normal(write_stream_t stream, pom_grp_entry_meta_t entry, cfg_t entry_cfg) {
    stream_printf(stream, "    ");
    stream_toupper(stream, dr_meta_name(pom_grp_entry_meta_normal_meta(entry)));
    stream_printf(stream, " const & %s(void) const { return normalEntry<", pom_grp_entry_meta_name(entry));
    stream_toupper(stream, dr_meta_name(pom_grp_entry_meta_normal_meta(entry)));
    stream_printf(stream, ">(%d); }\n", pom_grp_entry_meta_index(entry));
}

static void pom_tool_do_generate_hpp_read_ops_list(write_stream_t stream, pom_grp_entry_meta_t entry, cfg_t entry_cfg) {
    char typename[128];
    const char * entry_name = pom_grp_entry_meta_name(entry);
    int entry_idx = pom_grp_entry_meta_index(entry);

    cpe_str_dup(typename, sizeof(typename), dr_meta_name(pom_grp_entry_meta_list_meta(entry)));
    cpe_str_toupper(typename);

    stream_printf(
        stream, "    uint16_t %sCount(void) const { return listEntryCount(%d); }\n",
        entry_name, entry_idx);

    stream_printf(
        stream, "    uint16_t %sCapacity(void) const { return listEntryCapacity(%d); }\n"
        , entry_name, entry_idx);

    stream_printf(
        stream, "    %s const & %sAt(uint16_t pos) const { return listEntryAt<%s>(%d, pos); } \n",
        typename, entry_name, typename, entry_idx);

    stream_printf(
        stream, "    %s const * %sLSearch(%s const & key, int (*cmp)(%s const & l, %s const & r)) const { return listEntryLSearch<%s>(%d, key, cmp); } \n",
        typename, entry_name, typename, typename, typename, typename, entry_idx);

    stream_printf(stream, "    template<typename EqualT, typename KeyT>\n");
    stream_printf(
        stream, "    %s const * %sLSearch(KeyT key, EqualT const & cmp = EqualT()) const { return listEntryLSearch<%s, EqualT, KeyT>(%d, key, cmp); }\n", 
        typename, entry_name, typename, entry_idx);

    stream_printf(stream, "    template<typename EqualT, typename KeyT>\n");
    stream_printf(
        stream, "    int16_t %sLSearchPos(KeyT key, EqualT const & cmp = EqualT()) const { return listEntryLSearchPos<%s, EqualT, KeyT>(%d, key, cmp); }\n", 
        entry_name, typename, entry_idx);
}

static void pom_tool_do_generate_hpp_read_ops_ba(write_stream_t stream, pom_grp_entry_meta_t entry, cfg_t entry_cfg) {
    const char * entry_name = pom_grp_entry_meta_name(entry);
    int entry_idx = pom_grp_entry_meta_index(entry);

    stream_printf(stream, "    enum { ");
    stream_toupper(stream, entry_name);
    stream_printf(stream, "_BA_BYTES = %d };\n", pom_grp_entry_meta_ba_bytes(entry));

    stream_printf(stream, "    enum { ");
    stream_toupper(stream, entry_name);
    stream_printf(stream, "_BA_BITS = %d };\n", pom_grp_entry_meta_ba_bits(entry));

    stream_printf(
        stream, "    size_t %sCount(void) const { return baEntryCount(%d); }\n",
        entry_name, entry_idx);
    
    stream_printf(
        stream, "    cpe_ba_value_t %sGet(uint32_t pos) const { return baEntryGet(%d, pos); }\n",
        entry_name, entry_idx);

    stream_printf(
        stream, "    void  %sBinaryGet(void * data, uint32_t capacity) const { return baEntryBinaryGet(%d, data, capacity); }\n",
        entry_name, entry_idx);
}

static void pom_tool_do_generate_hpp_read_ops_binary(write_stream_t stream, pom_grp_entry_meta_t entry, cfg_t entry_cfg) {
    const char * entry_name = pom_grp_entry_meta_name(entry);
    int entry_idx = pom_grp_entry_meta_index(entry);

    const char * data_type = cfg_get_string(entry_cfg, "data-type", NULL);
    if (data_type) {
        stream_printf(
            stream, "    %s const &  %s(void) const { char _check_type_size[sizeof(%s) > %d ? -1 : 1]; (void)_check_type_size; return binaryEntry<%s >(%d); }\n",
            data_type, entry_name, data_type, pom_grp_entry_meta_binary_capacity(entry), data_type, entry_idx);
    }
    else {
        stream_printf(
            stream, "    void const *  _%s(void) const { return binaryEntry(%d); }\n",
            entry_name, entry_idx);
    }
}

static void pom_tool_do_generate_hpp_read_ops(write_stream_t stream, struct pom_tool_env * env) {
    pom_grp_entry_meta_t entry;
    struct pom_grp_entry_meta_it entry_it;

    pom_grp_entry_meta_it_init(env->m_pom_grp_meta, &entry_it);

    while((entry = pom_grp_entry_meta_next(&entry_it))) {
        cfg_t entry_cfg = pom_tool_do_generate_hpp_get_entry_cfg(env, pom_grp_entry_meta_name(entry));

        stream_printf(stream, "\n");

        switch(pom_grp_entry_meta_type(entry)) {
        case pom_grp_entry_type_normal:
            pom_tool_do_generate_hpp_read_ops_normal(stream, entry, entry_cfg);
            break;
        case pom_grp_entry_type_list:
            pom_tool_do_generate_hpp_read_ops_list(stream, entry, entry_cfg);
            break;
        case pom_grp_entry_type_ba:
            pom_tool_do_generate_hpp_read_ops_ba(stream, entry, entry_cfg);
            break;
        case pom_grp_entry_type_binary:
            pom_tool_do_generate_hpp_read_ops_binary(stream, entry, entry_cfg);
            break;
        default:
            break;
        }
    }
}

static void pom_tool_do_generate_hpp_write_ops_normal(write_stream_t stream, pom_grp_entry_meta_t entry, cfg_t entry_cfg) {
    stream_printf(stream, "    ");
    stream_toupper(stream, dr_meta_name(pom_grp_entry_meta_normal_meta(entry)));
    stream_printf(stream, "  & _%s(void) { return normalEntry<", pom_grp_entry_meta_name(entry));
    stream_toupper(stream, dr_meta_name(pom_grp_entry_meta_normal_meta(entry)));
    stream_printf(stream, ">(%d); }\n", pom_grp_entry_meta_index(entry));
}

static void pom_tool_do_generate_hpp_write_ops_list(write_stream_t stream, pom_grp_entry_meta_t entry, cfg_t entry_cfg) {
    char typename[128];
    const char * entry_name = pom_grp_entry_meta_name(entry);
    int entry_idx = pom_grp_entry_meta_index(entry);

    cpe_str_dup(typename, sizeof(typename), dr_meta_name(pom_grp_entry_meta_list_meta(entry)));
    cpe_str_toupper(typename);

    stream_printf(
        stream, "    %s & _%sAt(uint16_t pos) { return listEntryAt<%s>(%d, pos); } \n",
        typename, entry_name, typename, entry_idx);

    stream_printf(
        stream, "    void _%sAppend(%s const & data) { listEntryAppend(%d, data); } \n",
        entry_name, typename, entry_idx);

    stream_printf(
        stream, "    void _%sInsert(uint16_t pos, %s const & data) { listEntryInsert(%d, pos, data); } \n",
        entry_name, typename, entry_idx);

    stream_printf(
        stream, "    bool _%sRemove(uint16_t pos) { return listEntryRemove(%d, pos); } \n",
        entry_name, entry_idx);

    stream_printf(
        stream, "    void _%sClear(void) { listEntryClear(%d); } \n",
        entry_name, entry_idx);

    stream_printf(
        stream, "    %s * _%sLSearch(%s const & key, int (*cmp)(%s const & l, %s const & r)) { return listEntryLSearch<%s>(%d, key, cmp); } \n",
        typename, entry_name, typename, typename, typename, typename, entry_idx);

    stream_printf(stream, "    template<typename EqualT, typename KeyT>\n");
    stream_printf(
        stream, "    %s * _%sLSearch(KeyT key, EqualT const & cmp = EqualT()) { return listEntryLSearch<%s, EqualT, KeyT>(%d, key, cmp); }\n", 
        typename, entry_name, typename, entry_idx);
}

static void pom_tool_do_generate_hpp_write_ops_ba(write_stream_t stream, pom_grp_entry_meta_t entry, cfg_t entry_cfg) {
    const char * entry_name = pom_grp_entry_meta_name(entry);
    int entry_idx = pom_grp_entry_meta_index(entry);

    stream_printf(
        stream, "    void _%sSet(uint32_t pos, cpe_ba_value_t value) { baEntrySet(%d, pos, value); }\n",
        entry_name, entry_idx);

    stream_printf(
        stream, "    void  _%sBinarySet(void * data, uint32_t capacity) const { return baEntryBinaryGet(%d, data, capacity); }\n",
        entry_name, entry_idx);
}

static void pom_tool_do_generate_hpp_write_ops_binary(write_stream_t stream, pom_grp_entry_meta_t entry, cfg_t entry_cfg) {
    const char * entry_name = pom_grp_entry_meta_name(entry);
    int entry_idx = pom_grp_entry_meta_index(entry);

    const char * data_type = cfg_get_string(entry_cfg, "data-type", NULL);
    if (data_type) {
        stream_printf(
            stream, "    %s &  _%s(void) { char _check_type_size[sizeof(%s) > %d ? -1 : 1]; (void)_check_type_size; return binaryEntry<%s >(%d); }\n",
            data_type, entry_name, data_type, pom_grp_entry_meta_binary_capacity(entry), data_type, entry_idx);
    }
    else {
        stream_printf(
            stream, "    void *  _%s(void) { return binaryEntry(%d); }\n",
            entry_name, entry_idx);
    }
}

static void pom_tool_do_generate_hpp_write_ops(write_stream_t stream, struct pom_tool_env * env) {
    pom_grp_entry_meta_t entry;
    struct pom_grp_entry_meta_it entry_it;

    pom_grp_entry_meta_it_init(env->m_pom_grp_meta, &entry_it);

    while((entry = pom_grp_entry_meta_next(&entry_it))) {
        cfg_t entry_cfg = pom_tool_do_generate_hpp_get_entry_cfg(env, pom_grp_entry_meta_name(entry));

        stream_printf(stream, "\n");

        switch(pom_grp_entry_meta_type(entry)) {
        case pom_grp_entry_type_normal:
            pom_tool_do_generate_hpp_write_ops_normal(stream, entry, entry_cfg);
            break;
        case pom_grp_entry_type_list:
            pom_tool_do_generate_hpp_write_ops_list(stream, entry, entry_cfg);
            break;
        case pom_grp_entry_type_ba:
            pom_tool_do_generate_hpp_write_ops_ba(stream, entry, entry_cfg);
            break;
        case pom_grp_entry_type_binary:
            pom_tool_do_generate_hpp_write_ops_binary(stream, entry, entry_cfg);
            break;
        default:
            break;
        }
    }
}

static void pom_tool_do_generate_hpp(write_stream_t stream, struct pom_tool_env * env, const char * classname, const char * namespace) {
    assert(stream);
    assert(env);
    assert(env->m_pom_grp_meta);

    stream_printf(stream, "#ifndef POM_GRP_GENERATED_H");
    pom_tool_do_generate_hpp_print_ns(stream, namespace, pom_tool_do_generate_hpp_print_ns_upcase, "_", "");
    stream_printf(stream, "_");
    stream_toupper(stream, classname);
    stream_printf(stream, "_INCLEDED\n");
    stream_printf(stream, "#define POM_GRP_GENERATED_H");
    pom_tool_do_generate_hpp_print_ns(stream, namespace, pom_tool_do_generate_hpp_print_ns_upcase, "_", "");
    stream_printf(stream, "_");
    stream_toupper(stream, classname);
    stream_printf(stream, "_INCLEDED\n");
    stream_printf(stream, "#include \"cpepp/pom_grp/ObjectRef.hpp\"\n");
    stream_printf(stream, "\n");

    stream_printf(stream, "#ifdef _MSC_VER\n");
    stream_printf(stream, "# pragma warning(push)\n");
    stream_printf(stream, "# pragma warning(disable:4344)\n");
    stream_printf(stream, "#endif\n");
    stream_printf(stream, "\n");

    pom_tool_do_generate_hpp_print_ns(stream, namespace, pom_tool_do_generate_hpp_print_ns_normal, "namespace ", " {");
    stream_printf(stream, "\n");
    stream_printf(stream, "\n");

    stream_printf(stream, "class %s : public Cpe::PomGrp::ObjectRef {\n", classname);
    stream_printf(stream, "public:\n");
    stream_printf(stream, "    %s(pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj)\n", classname);
    stream_printf(stream, "        : Cpe::PomGrp::ObjectRef(obj_mgr, obj)\n");
    stream_printf(stream, "    {\n");
    stream_printf(stream, "    }\n");

    pom_tool_do_generate_hpp_read_ops(stream, env);    
    stream_printf(stream, "\n");

    stream_printf(stream, "protected:\n");
    pom_tool_do_generate_hpp_write_ops(stream, env);    
    stream_printf(stream, "\n");

    stream_printf(stream, "};\n");
    stream_printf(stream, "\n");
    pom_tool_do_generate_hpp_print_ns(stream, namespace, pom_tool_do_generate_hpp_print_ns_none, "", "}");
    stream_printf(stream, "\n");
    stream_printf(stream, "\n");

    stream_printf(stream, "#ifdef _MSC_VER\n");
    stream_printf(stream, "# pragma warning(pop)\n");
    stream_printf(stream, "#endif\n");
    stream_printf(stream, "\n");

    stream_printf(stream, "#endif\n");
}

int pom_tool_generate_hpp(struct pom_tool_env * env, const char * filename, const char * classname, const char * namespace) {
    struct write_stream_file stream;
    FILE * fp;

    if (env->m_pom_grp_meta == NULL) {
        CPE_ERROR(env->m_em, "generate hpp: no pom-meta!");
        return -1;
    }

    fp = file_stream_open(filename, "w", env->m_em);
    if (fp == NULL) {
        CPE_ERROR(env->m_em, "open %s fro generate lib c fail!", filename);
        return -1;
    }

    write_stream_file_init(&stream, fp, env->m_em);

    pom_tool_do_generate_hpp((write_stream_t)&stream, env, classname, namespace);

    file_stream_close(fp, env->m_em);

    return 0;
}


