#ifndef CPEPP_CFG_NODE_H
#define CPEPP_CFG_NODE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpe/cfg/cfg.h"
#include "System.hpp"
#include "NodePlacehold.hpp"
#include "NodeIterator.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Cfg {

class Node : public Cpe::Utils::SimulateObject {
public:
    operator cfg_t (void) const { return (cfg_t)this; }
    bool isValid(void) const { void const * p = (void const *)this; return p != NULL; }

    Utils::CString const & name(void) const { return Utils::CString::_cast(cfg_name(*this)); }
    bool isValue(void) const { return cfg_is_value(*this) ? true : false; }
    int type(void) const { return cfg_type(*this); }

    Node & parent(void) { return *(Node *)cfg_parent(*this); }
    Node const & parent(void) const { return *(Node const *)cfg_parent(*this); }

    Node & operator[](int pos) { return *((Node*)cfg_seq_at(*this, pos)); }
    Node const & operator[](int pos) const { return *((Node*)cfg_seq_at(*this, pos)); }
    NodePlacehold append(void) { return NodePlacehold(*this, "[]", NULL); }

    NodePlacehold operator[](const char * path) { return NodePlacehold(*this, path, cfg_find_cfg(*this, path)); }
    Node const & operator[](const char * path) const { return *((Node*)cfg_find_cfg(*this, path)); }

    Node * findChild(const char * path) { return (Node*)cfg_find_cfg(*this, path); }
    Node const * findChild(const char * path) const { return (Node*)cfg_find_cfg(*this, path); }

    Node & onlyChild(void);
    Node const & onlyChild(void) const;

    size_t childCount(void) const { return (size_t)cfg_child_count(*this); }

    void write(write_stream_t stream) const;
    bool tryWrite(write_stream_t stream, error_monitor_t em = NULL) { return cfg_yaml_write(stream, *this, em) == 0 ? true : false; }

    NodeConstIterator childs(void) const {
        NodeConstIterator r;
        cfg_it_init(&r.m_it, *this);
        return r;
    }

    NodeIterator childs(void) {
        NodeIterator r;
        cfg_it_init(&r.m_it, *this);
        return r;
    }

    operator int8_t(void) const;
    operator uint8_t(void) const;
    operator int16_t(void) const;
    operator uint16_t(void) const;
    operator int32_t(void) const;
    operator uint32_t(void) const;
    operator int64_t(void) const;
    operator uint64_t(void) const;
    operator float(void) const;
    operator double(void) const;
    operator const char * (void) const;

    int8_t dft(int8_t dft) const;
    uint8_t dft(uint8_t dft) const;
    int16_t dft(int16_t dft) const;
    uint16_t dft(uint16_t dft) const;
    int32_t dft(int32_t dft) const;
    uint32_t dft(uint32_t dft) const;
    int64_t dft(int64_t dft) const;
    uint64_t dft(uint64_t dft) const;
    float dft(float dft) const;
    double dft(double dft) const;
    const char * dft(const char * dft) const;

    int8_t asInt8(void) const { return *this; }
    uint8_t asUInt8(void) const { return *this; }
    int16_t asInt16(void) const { return *this; }
    uint16_t asUInt16(void) const { return *this; }
    int32_t asInt32(void) const { return *this; }
    uint32_t asUInt32(void) const { return *this; }
    int64_t asInt64(void) const { return *this; }
    uint64_t asUInt64(void) const { return *this; }
    float asFloat(void) const { return *this; }
    double asDouble(void) const { return *this; }
    Utils::CString const & asString(void) const { 
        return Utils::CString::_cast((const char *)(*this));
    }

    int8_t asInt8(int8_t v) const { return dft(v); }
    uint8_t asUInt8(uint8_t v) const { return dft(v); }
    int16_t asInt16(int16_t v) const { return dft(v); }
    uint16_t asUInt16(uint16_t v) const { return dft(v); }
    int32_t asInt32(int32_t v) const { return dft(v); }
    uint32_t asUInt32(uint32_t v) const { return dft(v); }
    int64_t asInt64(int64_t v) const { return dft(v); }
    uint64_t asUInt64(uint64_t v) const { return dft(v); }
    float asFloat(float v) const { return dft(v); }
    double asDouble(double v) const { return dft(v); }
    Utils::CString const & asString(const char * v) const { 
        return Utils::CString::_cast((const char *)(dft(v)));
    }

    static Node const & invalid(void) { return *(Node const *)0; }

    static Node & _cast(cfg_t cfg) { return  *(Node *)cfg; }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
