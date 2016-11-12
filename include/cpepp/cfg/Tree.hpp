#ifndef CPEPP_CFG_TREE_H
#define CPEPP_CFG_TREE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "Node.hpp"

namespace Cpe { namespace Cfg {

class Tree : public Cpe::Utils::Noncopyable {
public:
    operator cfg_t (void) const { return const_cast<cfg_t>(m_root); }

    explicit Tree(mem_allocrator_t alloc = 0);
    explicit Tree(const char * def, mem_allocrator_t alloc = 0);
    explicit Tree(cfg_t cfg, mem_allocrator_t alloc = 0);
    ~Tree();

    Node & root(void) { return Node::_cast(m_root); }
    Node const & root(void) const { return Node::_cast(m_root); }

private:
    cfg_t m_root;
};

}}

#endif
