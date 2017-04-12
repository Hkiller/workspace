#include <limits>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include "cpe/nm/nm_manage.h"
#include "cpepp/nm/Object.hpp"
#include "cpepp/nm/Manager.hpp"

namespace Cpe { namespace Nm {

extern struct nm_node_type g_object_type;

ObjectIterator Manager::objects(void) {
    ObjectIterator it;
    if (nm_mgr_nodes(&it._it, *this) != 0) {
        throw ::std::runtime_error("get named objects of Manager fail!");
    }
    it._next = it.next_i();
    return it;
}

ConstObjectIterator Manager::objects(void) const {
    ConstObjectIterator it;
    if (nm_mgr_nodes(&it._it, *this) != 0) {
        throw ::std::runtime_error("get named objects of Manager fail!");
    }
    it._next = it.next_i();
    return it;
}

Object const *
Manager::findObject(cpe_hash_string_t name) const {
    nm_node_t node = nm_mgr_find_node(*this, name);
    if (node == NULL
        || nm_node_type(node) != &g_object_type)
    {
        return NULL;
    }

    return Object::_cast(node);
}

Object *
Manager::findObject(cpe_hash_string_t name) {
    nm_node_t node = nm_mgr_find_node(*this, name);
    if (node == NULL
        || nm_node_type(node) != &g_object_type)
    {
        return NULL;
    }

    return Object::_cast(node);
}

Object const & Manager::object(cpe_hash_string_t name) const {
    Object const * r = findObject(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "named object " << cpe_hs_data(name) << " not exist!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Object & Manager::object(cpe_hash_string_t name) {
    Object * r = findObject(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "named object " << cpe_hs_data(name) << " not exist!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

bool Manager::removeObject(cpe_hash_string_t name) {
    nm_node_t node = nm_mgr_find_node(*this, name);
    if (node) {
        nm_node_free(node);
        return true;
    }
    else {
        return false;
    }
}

bool Manager::removeObject(const char * name) {
    nm_node_t node = nm_mgr_find_node_nc(*this, name);
    if (node) {
        nm_node_free(node);
        return true;
    }
    else {
        return false;
    }
}

Object const * Manager::findObjectNc(const char * name) const {
    nm_node_t node = nm_mgr_find_node_nc(*this, name);
    if (node == NULL
        || nm_node_type(node) != &g_object_type)
    {
        return NULL;
    }

    return Object::_cast(node);
}

Object * Manager::findObjectNc(const char * name) {
    nm_node_t node = nm_mgr_find_node_nc(*this, name);
    if (node == NULL
        || nm_node_type(node) != &g_object_type)
    {
        return NULL;
    }

    return Object::_cast(node);
}

Object const & Manager::objectNc(const char * name) const {
    Object const * r = findObjectNc(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "named object " << name << " not exist!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Object & Manager::objectNc(const char * name) {
    Object * r = findObjectNc(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "named object " << name << " not exist!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

}}
