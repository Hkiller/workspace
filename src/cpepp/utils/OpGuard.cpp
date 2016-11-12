#include <limits>
#include "cpepp/utils/OpGuard.hpp"

namespace Cpe { namespace Utils {

OpGuard::OpGuard() : m_needExecute(true) {
}

OpGuard::~OpGuard() {
    for(::std::vector<OpGuardNode*>::reverse_iterator it = m_ops.rbegin();
        it != m_ops.rend();
        ++it)
    {
        if (m_needExecute) {
            try {
                (*it)->execute();
            }
            catch(...) {}
        }

        delete *it;
    }

    m_ops.clear();
}

void OpGuard::addOpNode(::std::auto_ptr<OpGuardNode> node) {
    m_ops.reserve(m_ops.size() + 1);
    m_ops.push_back(node.get());
    node.release();
}

void OpGuard::releaseControl(void) {
    m_needExecute = false;
}

OpGuard::OpGuardNode::~OpGuardNode() {
}

}}
