#ifndef CPEPP_POM_GRP_OBJECT_H
#define CPEPP_POM_GRP_OBJECT_H
#include <stdexcept>
#include <cassert>
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "System.hpp"

namespace Cpe { namespace PomGrp {

class ObjectRef {
public:
    ObjectRef(pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj)
        : m_obj_mgr(obj_mgr)
        , m_obj(obj)
    {
    }

    ObjectRef(ObjectRef const & o)
        : m_obj_mgr(o.m_obj_mgr)
        , m_obj(o.m_obj)
    {
    }

    bool isValid(void) const { return m_obj != NULL; }

    /*normal entries*/
    template<typename T>
    T & normalEntry(int entryPos) {
        void * buf = pom_grp_obj_normal_check_or_create_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
        if (buf == NULL) throw ::std::bad_alloc();
        return *(T*)buf;
    }

    template<typename T>
    T const & normalEntry(int entryPos) const {
        void * buf = pom_grp_obj_normal_check_or_create_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
        if (buf == NULL) throw ::std::bad_alloc();
        return *(T const *)buf;
    }

    template<typename T>
    T & normalEntry(const char * entryName) {
        void * buf = pom_grp_obj_normal_check_or_create(m_obj_mgr, m_obj, entryName);
        if (buf == NULL) throw ::std::bad_alloc();
        return *(T*)buf;
    }

    template<typename T>
    T const & normalEntry(const char * entryName) const {
        void * buf = pom_grp_obj_normal_check_or_create(m_obj_mgr, m_obj, entryName);
        if (buf == NULL) throw ::std::bad_alloc();
        return *(T const *)buf;
    }

    /*list entries*/
    uint16_t listEntryCount(int entryPos) const {
        return pom_grp_obj_list_count_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
    }

    uint16_t listEntryCapacity(int entryPos) const {
        return pom_grp_obj_list_capacity_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
    }

    template<typename T>
    T const & listEntryAt(int entryPos, uint16_t listPos) const {
        void * buf = pom_grp_obj_list_at_ex(m_obj_mgr, m_obj, entry_meta(entryPos), listPos);
        if (buf == NULL) throw ::std::runtime_error("listEntryAt fail!");
        return *(T const *) buf;
    }

    template<typename T>
    T & listEntryAt(int entryPos, uint16_t listPos) {
        void * buf = pom_grp_obj_list_at_ex(m_obj_mgr, m_obj, entry_meta(entryPos), listPos);
        if (buf == NULL) throw ::std::runtime_error("listEntryAt fail!");
        return *(T *) buf;
    }

    template<typename T>
    void listEntryAppend(int entryPos, T const & data) {
        this->listEntryAppend(entryPos, (void const *)&data);
    }

    template<typename T>
    bool listEntryTryAppend(int entryPos, T const & data) {
        return this->listEntryTryAppend(entryPos, (void const *)&data);
    }

    void listEntryAppend(int entryPos, void const * data) {
        if (pom_grp_obj_list_append_ex(m_obj_mgr, m_obj, entry_meta(entryPos), (void*)data) != 0) {
            throw ::std::runtime_error("listEntryAppend fail!");
        }
    }

    bool listEntryTryAppend(int entryPos, void const * data) {
        return pom_grp_obj_list_append_ex(m_obj_mgr, m_obj, entry_meta(entryPos), (void*)data) == 0;
    }

    template<typename T>
    void listEntryInsert(int entryPos, uint16_t insertPos, T const & data) {
        this->listEntryInsert(entryPos, insertPos, (void const *)&data);
    }

    void listEntryInsert(int entryPos, uint16_t insertPos, void const * data) {
        if (pom_grp_obj_list_insert_ex(m_obj_mgr, m_obj, entry_meta(entryPos), insertPos, (void*)data) != 0) {
            throw ::std::runtime_error("listEntryInsert fail!");
        }
    }

    bool listEntryRemove(int entryPos, uint16_t listPos) {
        return pom_grp_obj_list_remove_ex(m_obj_mgr, m_obj, entry_meta(entryPos), listPos) == 0;
    }

    void listEntryClear(int entryPos) {
        if (pom_grp_obj_list_clear_ex(m_obj_mgr, m_obj, entry_meta(entryPos)) != 0) {
            throw ::std::runtime_error("listEntryClear fail!");
        }
    }

    template<typename T>
    T const * listEntryLSearch(int entryPos, T const data, int (*cmp)(T const & l, T const & r)) const {
        typedef int (*cmp_t)(void const * l, void const * r);

        return (T const *)listEntryLSearch(entryPos, (const void *)&data, (cmp_t)cmp);
    }

    void const * listEntryLSearch(int entryPos, void const * data, int (*cmp)(void const * l, void const * r)) const {
        return pom_grp_obj_list_lsearch_ex(m_obj_mgr, m_obj, entry_meta(entryPos), data, cmp);
    }

    template<typename T>
    T * listEntryLSearch(int entryPos, T const data, int (*cmp)(T const & l, T const & r)) {
        typedef int (*cmp_t)(void const * l, void const * r);

        return (T *)listEntryLSearch(entryPos, (const void *)&data, (cmp_t)cmp);
    }

    void * listEntryLSearch(int entryPos, void const * data, int (*cmp)(void const * l, void const * r)) {
        return pom_grp_obj_list_lsearch_ex(m_obj_mgr, m_obj, entry_meta(entryPos), data, cmp);
    }

    template<typename T, typename EqualT, typename KeyT>
    T const * listEntryLSearch(int entryPos, KeyT key, EqualT const & cmp = EqualT()) const {
        uint16_t c = listEntryCount(entryPos);
        for(uint16_t i = 0; i < c; ++i) {
            T const & v = listEntryAt<T>(entryPos, i);
            if (cmp(v, key)) return &v;
        }

        return NULL;
    }

    template<typename T, typename EqualT, typename KeyT>
    T * listEntryLSearch(int entryPos, KeyT key, EqualT const & cmp = EqualT()) const {
        uint16_t c = listEntryCount(entryPos);
        for(uint16_t i = 0; i < c; ++i) {
            T & v = listEntryAt<T>(entryPos, i);
            if (cmp(v, key)) return &v;
        }

        return NULL;
    }

    template<typename T, typename EqualT, typename KeyT>
    int16_t listEntryLSearchPos(int entryPos, KeyT key, EqualT const & cmp = EqualT()) const {
        uint16_t c = listEntryCount(entryPos);
        for(uint16_t i = 0; i < c; ++i) {
            T const & v = listEntryAt<T>(entryPos, i);
            if (cmp(v, key)) return i;
        }

        return -1;
    }

    /*ba entries*/
    uint16_t baEntryCount(int entryPos) const {
        return pom_grp_obj_ba_bit_count_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
    }

    cpe_ba_value_t baEntryGet(int entryPos, uint32_t baPos) const {
        return pom_grp_obj_ba_get_ex(m_obj_mgr, m_obj, entry_meta(entryPos), baPos);
    }

    void baEntryBinaryGet(int entryPos, void * data, uint32_t capacity) const {
        if (pom_grp_obj_ba_get_binary_ex(m_obj_mgr, m_obj, entry_meta(entryPos), data, capacity) != 0) {
            throw ::std::runtime_error("baEntryBinaryGet fail!");
        }
    }

    void baEntrySet(int entryPos, uint32_t baPos, cpe_ba_value_t value) {
        if (pom_grp_obj_ba_set_ex(m_obj_mgr, m_obj, entry_meta(entryPos), baPos, value) != 0) {
            throw ::std::runtime_error("baEntrySet fail!");
        }
    }

    void baEntryBinarySet(int entryPos, void const * data, uint32_t capacity) const {
        if (pom_grp_obj_ba_set_binary_ex(m_obj_mgr, m_obj, entry_meta(entryPos), data, capacity) != 0) {
            throw ::std::runtime_error("baEntryBinarySet fail!");
        }
    }

    /*binary entries*/
    uint16_t binaryEntryCapacity(int entryPos) const {
        return pom_grp_obj_binary_capacity_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
    }

    void * binaryEntry(int entryPos) const {
        return pom_grp_obj_binary_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
    }

    void * binaryEntryCheckCreate(int entryPos) const {
        return pom_grp_obj_binary_check_or_create_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
    }

    void binaryEntrySet(int entryPos, void const * data, size_t capacity) {
        if (pom_grp_obj_binary_set_ex(m_obj_mgr, m_obj, entry_meta(entryPos), data, capacity) != 0) {
            throw ::std::runtime_error("binaryEntrySet fail!");
        }
    }

    template<typename T>
    T & binaryEntry(int entryPos) {
        assert(sizeof(T) <= binaryEntryCapacity(entryPos));
        
        void * buf = pom_grp_obj_binary_check_or_create_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
        if (buf == NULL) throw ::std::bad_alloc();
        return *(T*)buf;
    }

    template<typename T>
    T const & binaryEntry(int entryPos) const {
        assert(sizeof(T) <= binaryEntryCapacity(entryPos));
        
        void const * buf = pom_grp_obj_binary_check_or_create_ex(m_obj_mgr, m_obj, entry_meta(entryPos));
        if (buf == NULL) throw ::std::bad_alloc();
        return *(T const *)buf;
    }

    /*common operations*/
    pom_grp_entry_meta_t entry_meta(int entryPos) const {
        return pom_grp_entry_meta_at(pom_grp_obj_mgr_meta(m_obj_mgr), entryPos);
    }

    pom_grp_obj_t _obj(void) { return m_obj; }
    pom_grp_obj_mgr_t _obj_mgr(void) { return m_obj_mgr; }
    void _free(void) { if (m_obj) { pom_grp_obj_free(m_obj_mgr, m_obj); m_obj = 0;} }

    operator pom_grp_obj_t() { return m_obj; }

private:
    pom_grp_obj_mgr_t m_obj_mgr;
    pom_grp_obj_t m_obj;
};

}}

#endif
