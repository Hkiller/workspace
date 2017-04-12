#include <limits>
#include <sstream>
#include "cpe/utils/string_utils.h"
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpe/cfg/cfg_manage.h"
#include "cpepp/dr/Exceptions.hpp"
#include "cpepp/cfg/NodePlacehold.hpp"
#include "cpepp/cfg/Node.hpp"
#include "cpepp/cfg/Exceptions.hpp"

namespace Cpe { namespace Cfg {

cfg_t ConstNodePlacehold::to_node(cpe_str_buf_t buf) const {
    if (m_parent_n) {
        cpe_str_buf_cpy(buf, m_path);
        return *m_parent_n;
    }
    else if (m_parent_p) {
        cfg_t r = m_parent_p->to_node(buf);
        if (r) {
            if (m_path[0] != '[') cpe_str_buf_cat(buf, ".");

            cpe_str_buf_cat(buf, m_path);
        }
        return r;
    }
    else {
        return NULL;
    }
}

#define CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(__type_t, __type)                         \
ConstNodePlacehold::operator __type_t(void) const {                          \
    __type_t rv;                                                        \
    if (cfg_try_as_ ## __type (m_node, &rv) != 0) {                      \
        if (m_node == NULL) {                                             \
            throw invalid_cfg_node("Cfg Node is not valid!");           \
        }                                                               \
        else {                                                          \
            throw Dr::type_convert_error("Cfg Node read " #__type " fail!"); \
        }                                                               \
    }                                                                   \
                                                                        \
    return rv;                                                          \
}                                                                       \
__type_t ConstNodePlacehold::dft(__type_t v) const {                         \
    return cfg_as_ ## __type(m_node, v);                                 \
}

CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(int8_t, int8);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(uint8_t, uint8);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(int16_t, int16);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(uint16_t, uint16);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(int32_t, int32);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(uint32_t, uint32);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(int64_t, int64);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(uint64_t, uint64);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(float, float);
CPE_CFG_NODEPLACEHOLD_GEN_READ_TYPE(double, double);

ConstNodePlacehold::operator const char *(void) const {
    const char * r = cfg_as_string(m_node, NULL);
    if (r == NULL) {
        void const * p = (void const *)this;
        if (p == NULL) {
            throw invalid_cfg_node("Cfg Node is not valid!");
        }
        else {
            throw Dr::type_convert_error("Cfg Node read int8 fail!");
        }
    }
    return r;
}

const char * ConstNodePlacehold::dft(const char * dft) const {
    return cfg_as_string(m_node, dft); 
}

Node & NodePlacehold::createStruct(void) {
    char buf[256];
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(buf, sizeof(buf));
    cfg_t root;

    root = to_node(&str_buf);
    if (root == NULL) {
        ::std::ostringstream os;
        os << "root node not exist(" << buf << ")!";
        throw ::std::runtime_error(os.str());
    }

    if (cpe_str_buf_is_overflow(&str_buf)) { 
        ::std::ostringstream os;
        os << "path too long(" << buf << ")!";
        throw ::std::runtime_error(os.str());
    }

    Cpe::Utils::ErrorCollector ec;
    m_node = cfg_add_struct(root, buf, ec);
    if (m_node == NULL) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }

    return * (Node*)m_node;
}

Node & NodePlacehold::createSeq(void) {
    char buf[256];
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(buf, sizeof(buf));
    cfg_t root;

    root = to_node(&str_buf);
    if (root == NULL) {
        ::std::ostringstream os;
        os << "root node not exist(" << buf << ")!";
        throw ::std::runtime_error(os.str());
    }

    if (cpe_str_buf_is_overflow(&str_buf)) { 
        ::std::ostringstream os;
        os << "path too long(" << buf << ")!";
        throw ::std::runtime_error(os.str());
    }

    Cpe::Utils::ErrorCollector ec;
    m_node = cfg_add_seq(root, buf, ec);
    if (m_node == NULL) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }

    return * (Node*)m_node;
}

#define CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(__type_t, __type)              \
NodePlacehold & NodePlacehold::operator= (__type_t v) {                 \
    char buf[256];                                                      \
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(buf, sizeof(buf));    \
    cfg_t root;                                                         \
                                                                        \
    root = to_node(&str_buf);                                           \
    if (root == NULL) {                                                 \
        ::std::ostringstream os;                                        \
        os << "root node not exist(" << buf << ")!";                    \
        throw ::std::runtime_error(os.str());                           \
    }                                                                   \
                                                                        \
    if (cpe_str_buf_is_overflow(&str_buf)) {                            \
        ::std::ostringstream os;                                        \
        os << "path too long(" << buf << ")!";                          \
        throw ::std::runtime_error(os.str());                           \
    }                                                                   \
                                                                        \
    Cpe::Utils::ErrorCollector ec;                                      \
    m_node = cfg_add_ ## __type(root, buf, v, ec);                      \
    if (m_node == NULL) {                                               \
        ec.checkThrowWithMsg< ::std::runtime_error>();                  \
    }                                                                   \
                                                                        \
    return *this;                                                       \
}

CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(int8_t, int8);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(uint8_t, uint8);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(int16_t, int16);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(uint16_t, uint16);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(int32_t, int32);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(uint32_t, uint32);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(int64_t, int64);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(uint64_t, uint64);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(float, float);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(double, double);
CPE_CFG_NODEPLACEHOLD_GEN_ASSIGN(const char *, string);

}}
