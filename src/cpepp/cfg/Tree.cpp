#include <limits>
#include <stdexcept>
#include "cpe/utils/stream_mem.h"
#include "cpepp/cfg/Tree.hpp"

namespace Cpe { namespace Cfg {

static cfg_t parse(const char * def, mem_allocrator_t alloc) {
    if (def == 0) return 0;

    cfg_t root = cfg_create(alloc);
    if (root == 0) {
        throw ::std::runtime_error("construct Cpe::Cfg::Tree, alloc cft root fail!");
    }

    read_stream_mem inputStream = CPE_READ_STREAM_MEM_INITIALIZER(def, strlen(def));

    if (cfg_yaml_read(root, (read_stream_t)&inputStream, cfg_merge_use_new, 0) != 0) {
        cfg_free(root);
        throw ::std::runtime_error("construct Cpe::Cfg::Tree, parse cft fail!");
    }

    return root;
}

static cfg_t dump(cfg_t cfg, mem_allocrator_t alloc) {
    if (cfg == 0) return 0;

    cfg_t root = cfg_create(alloc);
    if (root == 0) {
        throw ::std::runtime_error("construct Cpe::Cfg::Tree, alloc cft root fail!");
    }

    if (cfg_merge(root, cfg, cfg_merge_use_new, 0) != 0) {
        cfg_free(root);
        throw ::std::runtime_error("construct Cpe::Cfg::Tree, dump cft fail!");
    }

    return root;
}

Tree::Tree(mem_allocrator_t alloc) 
    : m_root(cfg_create(alloc))
{
}

Tree::Tree(const char * def, mem_allocrator_t alloc)
    : m_root(parse(def, alloc))
{
}

Tree::Tree(cfg_t cfg, mem_allocrator_t alloc)
    : m_root(dump(cfg, alloc))
{
}

Tree::~Tree() {
    if (m_root) {
        cfg_free(m_root);
        m_root = 0;
    }
}

}}
