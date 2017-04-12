#ifndef CPEPP_CFG_NODEITERATOR_H
#define CPEPP_CFG_NODEITERATOR_H
#include "cpe/cfg/cfg.h"
#include "System.hpp"

namespace Cpe { namespace Cfg {

class NodeConstIterator {
public:
    NodeConstIterator();

    Node const * next(void) const { return (Node const *)cfg_it_next(&m_it); }

private:
    mutable cfg_it_t m_it;

friend class Node;
friend class ConstNodePlacehold;
friend class NodePlacehold;
friend class NodeIterator;
};

class NodeIterator : public NodeConstIterator {
public:
    using NodeConstIterator::next;

    Node * next(void) { return (Node *)cfg_it_next(&m_it); }
};

}}

#endif
