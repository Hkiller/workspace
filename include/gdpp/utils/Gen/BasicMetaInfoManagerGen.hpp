#ifndef GDPP_UTILS_GEN_BASICMETAINFOMANAGERGEN_H
#define GDPP_UTILS_GEN_BASICMETAINFOMANAGERGEN_H
#include <vector>
#include <algorithm>
#include <typeinfo>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_file.h"
#include "cpepp/cfg/Node.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Log.hpp"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/pal/pal_strings.h"

namespace Gd { namespace Utils {

template<typename BaseT, typename ElementT, typename Compare>
class BasicMetaInfoManagerGen : public BaseT {
public:
    typedef BasicMetaInfoManagerGen Base;

    virtual void load_pbuf_array(void const * data, size_t data_capacity) {
        ElementT buf;
        const char * rp = (const char *)data;
        int r_capacity = (int)data_capacity;

        while(r_capacity > 0) {
            int rv;
            size_t read_with_size;
 
            bzero(&buf, sizeof(buf));
            
            rv = dr_pbuf_read_with_size(&buf, sizeof(buf), rp, data_capacity, &read_with_size, Cpe::Dr::MetaTraits<ElementT>::META, NULL);
            if (rv < 0) {
                APP_ERROR("load from pbuf array fail, rv=%d!", rv);
                break;
            }

            m_elements.push_back(buf);

            rp += read_with_size;
            r_capacity -= read_with_size;
        }
    }

    virtual void load(ElementT const * data, size_t count) {
        size_t writeCount = m_elements.size();

        m_elements.resize(m_elements.size() + count);

        for(size_t i = 0; i < count; ++i) {
            m_elements[writeCount++] = data[i];
        }
    }

    virtual void load(Cpe::Cfg::Node const & configNode) {
        size_t writeCount = m_elements.size();
        size_t readCount = 0;

        m_elements.resize(m_elements.size() + configNode.childCount());

        Cpe::Cfg::NodeConstIterator configNodes = configNode.childs();
        while(Cpe::Cfg::Node const * boxCfg = configNodes.next()) {
            if (!Cpe::Dr::MetaTraits<ElementT>::META.try_load_from_cfg(m_elements[writeCount], *boxCfg)) {
                APP_ERROR("load %s fail, index=%d!", typeid(ElementT).name(), (int)readCount);
            }
            else {
                ++writeCount;
            }

            ++readCount;
        }

        m_elements.resize(writeCount);
    }

    virtual void dump(uint32_t level, write_stream_t stream) const {
        for(typename ElementContainer::const_iterator it = m_elements.begin();
            it != m_elements.end();
            ++it)
        {
            stream_putc_count(stream, ' ', level << 2);

            Cpe::Dr::MetaTraits<ElementT>::META.dump_data(stream, (void const *)&*it, sizeof(*it));

            stream_putc(stream, '\n');
        }
    }

    virtual size_t count(void) const {
        return m_elements.size();
    }

    virtual ElementT const & at(size_t pos) const {
        return m_elements[pos];
    }

    virtual void clear(void) { m_elements.clear(); }

    virtual void const * _buf(void) const { return &m_elements[0]; }
    virtual size_t _buf_size(void) const { return sizeof(m_elements[0]) * m_elements.size(); }

protected:
    void sort(void) {
        ::std::stable_sort(m_elements.begin(), m_elements.end(), Compare());
    }
    
    typedef ::std::vector<ElementT> ElementContainer;

    bool _is_equal(ElementT const & l, ElementT const & r) const {
        return !Compare()(l, r) && !Compare()(r, l);
    }

    ::std::pair<ElementT const *, ElementT const *>
    find_range(ElementT const & key) const {
        if (m_elements.empty()) {
            ::std::pair<ElementT const *, ElementT const *> r;
            r.first = NULL;
            r.second = NULL;
            return  r;
        }
        else {
            ElementT const * all_begin = &m_elements[0];
            ElementT const * all_end = all_begin + m_elements.size();
            return ::std::pair<ElementT const *, ElementT const *>(
                ::std::lower_bound(all_begin, all_end, key, Compare()),
                ::std::upper_bound(all_begin, all_end, key, Compare()));
        }
    }

    template<typename RangeT>
    RangeT find_range(ElementT const & key) const {
        if (m_elements.empty()) {
            RangeT r = { NULL, NULL };
            return r;
        }
        else {
            ElementT const * all_begin = &m_elements[0];
            ElementT const * all_end = all_begin + m_elements.size();
            RangeT r = {
                ::std::lower_bound(all_begin, all_end, key, Compare()),
                ::std::upper_bound(all_begin, all_end, key, Compare())
            };
            return r;
		}
    }

    template<typename RangeT, typename CmpT>
    RangeT find_range(ElementT const & key, CmpT cmp = CmpT()) const {
        if (m_elements.empty()) {
            RangeT r = { NULL, NULL };
            return r;
        }
        else {
            ElementT const * all_begin = &m_elements[0];
            ElementT const * all_end = all_begin + m_elements.size();
            RangeT r = {
                ::std::lower_bound(all_begin, all_end, key, cmp),
                ::std::upper_bound(all_begin, all_end, key, cmp)
            };
            return r;
        }
    }

    ElementT const * find_first(ElementT const & key) const {
        typename ElementContainer::const_iterator pos = 
            ::std::lower_bound(m_elements.begin(), m_elements.end(), key, Compare());
        if (pos != m_elements.end() && _is_equal(*pos, key)) {
            return &*pos;
        }
        else {
            return 0;
        }
    }

    ElementT const * upper_bound(ElementT const & key) const {
        typename ElementContainer::const_iterator pos = 
            ::std::upper_bound(m_elements.begin(), m_elements.end(), key, Compare());
        if (pos != m_elements.end()) {
            return &*pos;
        }
        else {
            return end();
        }
    }

    ElementT const * lower_bound(ElementT const & key) const {
        typename ElementContainer::const_iterator pos = 
            ::std::lower_bound(m_elements.begin(), m_elements.end(), key, Compare());
        if (pos != m_elements.end()) {
            return &*pos;
        }
        else {
            return end();
        }
    }

    template<typename CmpT>
    ElementT const * lineerFind(ElementT const & key, CmpT const & cmp = CmpT()) const {
        for(typename ElementContainer::const_iterator pos = m_elements.begin();
            pos != m_elements.end();
            ++pos)
        {
            if (cmp(*pos, key)) return &*pos;
        }

        return 0;
    }

    ElementT & at(size_t pos) {
        return m_elements[pos];
    }

    ElementT const * begin(void) const { return &m_elements[0]; }
    ElementT const * end(void) const { return begin() + m_elements.size(); }
    ElementT const * last(void) const { return m_elements.size() ? (end() - 1) : NULL; }

    ElementContainer m_elements;
};

}}

#endif
