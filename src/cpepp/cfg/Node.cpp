#include <limits>
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpepp/dr/Exceptions.hpp"
#include "cpepp/cfg/Node.hpp"
#include "cpepp/cfg/Exceptions.hpp"

namespace Cpe { namespace Cfg {

#define CPE_CFG_GEN_READ_TYPE(__type_t, __type)                         \
Node::operator __type_t(void) const {                                   \
    __type_t rv;                                                        \
    if (cfg_try_as_ ## __type (*this, &rv) != 0) {                      \
        if (this == NULL) {                                             \
            throw invalid_cfg_node("Cfg Node is not valid!");           \
        }                                                               \
        else {                                                          \
            throw Dr::type_convert_error("Cfg Node read " #__type " fail!"); \
        }                                                               \
    }                                                                   \
                                                                        \
    return rv;                                                          \
}                                                                       \
__type_t Node::dft(__type_t v) const {                                  \
    return cfg_as_ ## __type(*this, v);                                 \
}

CPE_CFG_GEN_READ_TYPE(int8_t, int8);
CPE_CFG_GEN_READ_TYPE(uint8_t, uint8);
CPE_CFG_GEN_READ_TYPE(int16_t, int16);
CPE_CFG_GEN_READ_TYPE(uint16_t, uint16);
CPE_CFG_GEN_READ_TYPE(int32_t, int32);
CPE_CFG_GEN_READ_TYPE(uint32_t, uint32);
CPE_CFG_GEN_READ_TYPE(int64_t, int64);
CPE_CFG_GEN_READ_TYPE(uint64_t, uint64);
CPE_CFG_GEN_READ_TYPE(float, float);
CPE_CFG_GEN_READ_TYPE(double, double);

Node::operator const char *(void) const {
    const char * r = cfg_as_string(*this, NULL);
    if (r == NULL) {
        void * p = (void *)this;
        if (p == NULL) {
            throw invalid_cfg_node("Cfg Node is not valid!");
        }
        else {
            throw Dr::type_convert_error("Cfg Node read int8 fail!");
        }
    }
    return r;
}

const char * Node::dft(const char * dft) const {
    return cfg_as_string(*this, dft); 
}

NodeConstIterator::NodeConstIterator() {
    m_it.next = NULL;
}

Node & Node::onlyChild(void) {
    NodeIterator childs = this->childs();
    Node * r = childs.next();
    if (r == 0)  {
        throw ::std::runtime_error("Node::onlyChild no child");
    }

    if (childs.next()) {
        throw ::std::runtime_error("Node::onlyChild too many child");
    }

    return *r;
}

Node const & Node::onlyChild(void) const {
    NodeConstIterator childs = this->childs();
    Node const * r = childs.next();
    if (r == 0)  {
        throw ::std::runtime_error("Node::onlyChild no child");
    }

    if (childs.next()) {
        throw ::std::runtime_error("Node::onlyChild too many child");
    }

    return *r;
}

void Node::write(write_stream_t stream) const {
    Cpe::Utils::ErrorCollector ec;
    if (cfg_yaml_write(stream, *this, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
}

}}
