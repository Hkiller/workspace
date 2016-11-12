#ifndef USFPP_LOGIC_USE_DYNAMICLIST_H
#define USFPP_LOGIC_USE_DYNAMICLIST_H
#include <cassert>
#include <algorithm>
#include "cpepp/utils/IntTypeSelect.hpp"
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/dr/Meta.hpp"
#include "usfpp/logic/LogicOpData.hpp"
#include "usfpp/logic/LogicOpContext.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "System.hpp"

namespace Usf { namespace Logic {

template<typename ListT, typename EleT>
class LogicOpDynList {
public:
    typedef typename Cpe::Utils::IntTypeSelect<sizeof(ListT) - sizeof(EleT)>::uint_type size_type;

    LogicOpDynList(
        LogicOpContext & context,
        size_t capacity = 8)
        : m_holder_type(0)
        , m_holder(&context)
    {
        checkCreateData(Cpe::Dr::MetaTraits<ListT>::META, capacity);
    }

    LogicOpDynList(
        Cpe::Dr::Meta const & meta,
        LogicOpContext & context,
        size_t capacity = 8)
        : m_holder_type(0)
        , m_holder(&context)
    {
        checkCreateData(Cpe::Dr::MetaTraits<ListT>::META, capacity);
    }

    LogicOpDynList(
        LogicOpStackNode & stackNode,
        size_t capacity = 8)
        : m_holder_type(1)
        , m_holder(&stackNode)
    {
        checkCreateData(Cpe::Dr::MetaTraits<ListT>::META, capacity);
    }

    LogicOpDynList(
        Cpe::Dr::Meta const & meta,
        LogicOpStackNode & stackNode,
        size_t capacity = 8)
        : m_holder_type(1)
        , m_holder(&stackNode)
    {
        checkCreateData(Cpe::Dr::MetaTraits<ListT>::META, capacity);
    }

    LogicOpDynList(
        LogicOpRequire & require,
        size_t capacity = 8)
        : m_holder_type(2)
        , m_holder(&require)
    {
        checkCreateData(Cpe::Dr::MetaTraits<ListT>::META, capacity);
    }

    LogicOpDynList(
        Cpe::Dr::Meta const & meta,
        LogicOpRequire & require,
        size_t capacity = 8)
        : m_holder_type(2)
        , m_holder(&require)
    {
        checkCreateData(Cpe::Dr::MetaTraits<ListT>::META, capacity);
    }

    size_type count(void) const { return *(size_type const *)m_data.get().data(); }
    size_type capacity(void) const { return (m_data.get().capacity() - sizeof(size_type)) / sizeof(EleT); }

    EleT & operator[] (size_type pos) { return data()[pos]; }
    EleT const & operator[] (size_type pos) const { return data()[pos]; }

    EleT & at(size_type pos) { 
        assert(pos < count());
        return data()[pos];
    }

    EleT const & at (size_type pos) const {
        assert(pos < count());

        return data()[pos];
    }

    void append(EleT const & e) {
        if (count() + 1 >= capacity()) {
            size_type inc_capacity = capacity();
            if (inc_capacity < 16) inc_capacity = 16;
            checkCreateData(m_data.get().meta(), capacity() + inc_capacity);
        }

        assert(count() + 1 < capacity());

        size_t pos = (*(size_type *)m_data.get().data()) ++;

        operator[](pos) = e;
    }

    void erase(size_type pos) {
        size_type count = this->count();
        assert(pos < count);

        if (pos + 1 < count) {
            memcpy(&data()[pos], &data()[count - 1], sizeof(EleT));
        }

        --(*(size_type *)m_data.get().data());
    }

    void clear(void) {
        (*(size_type *)m_data.get().data()) = 0;
    }

    template<typename CmpT>
    void sort(CmpT const & cmp = CmpT()) {
        EleT * begin = data();
        ::std::sort(begin, begin + count(), cmp);
    }

    template<typename CmpT>
    ::std::pair<EleT const *, EleT const *>
    find_range(EleT const & key, CmpT const & cmp) const {
        EleT const * begin = data();

        return ::std::pair<EleT const *, EleT const *>(
            ::std::lower_bound(begin, begin + count(), cmp),
            ::std::upper_bound(begin, begin + count(), cmp));
    }

    template<typename CmpT>
    ::std::pair<EleT *, EleT *>
    find_range(EleT const & key, CmpT const & cmp) {
        EleT * begin = data();

        return ::std::pair<EleT *, EleT *>(
            ::std::lower_bound(begin, begin + count(), cmp),
            ::std::upper_bound(begin, begin + count(), cmp));
    }


private:
    EleT * data(void) { return (EleT*)(((size_type *)m_data.get().data()) + 1); }
    EleT const * data(void) const { return (EleT const *)(((size_type const *)m_data.get().data()) + 1); }

    void checkCreateData(LPDRMETA meta, size_t capacity) {
        assert(m_holder_type < 3);

        if (m_holder_type == 0){
            m_data = ((LogicOpContext *)m_holder)->checkCreateData(meta, sizeof(EleT) * capacity + sizeof(size_type));
        }
        else if (m_holder_type == 1) {
            m_data = ((LogicOpStackNode *)m_holder)->checkCreateData(meta, sizeof(EleT) * capacity + sizeof(size_type));
        }
        else {
            m_data = ((LogicOpRequire *)m_holder)->checkCreateData(meta, sizeof(EleT) * capacity + sizeof(size_type));
        }

        assert(m_data.valid());
        if (!m_data.valid()) throw ::std::bad_alloc();
    }

    Cpe::Utils::ObjRef<LogicOpData> m_data;
    int m_holder_type;
    void * m_holder;
};

}}

#endif
