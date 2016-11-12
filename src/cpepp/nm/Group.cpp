#include <cassert>
#include <sstream>
#include <stdexcept>
#include "cpe/nm/nm_manage.h"
#include "cpepp/nm/Group.hpp"

namespace Cpe { namespace Nm {

Group::~Group() {
}

void * Group::operator new (size_t size, nm_mgr_t nmm, const char * name) {
    nm_node_t node = nm_group_create(nmm, name, size);
    if (node == NULL) {
        if (nm_mgr_find_node_nc(nmm, name)) {
            ::std::ostringstream os;
            os << "named object " << name << " already exist!";
            throw ::std::runtime_error(os.str());
        }
        else {
            throw ::std::bad_alloc();
        }
    }

    return nm_node_data(node);
}

void Group::destoryMembers(void) {
    nm_group_free_members(*this);
}

void Group::addMember(Object & object) {
    if (nm_group_add_member(*this, object) != 0) {
        ::std::ostringstream os;
        os << "add object " << object.name() << " to group " << name() << " fail!";
        throw ::std::runtime_error(os.str());
    }
}

ObjectIterator Group::members(void) {
    ObjectIterator it;
    if (nm_group_members(&it._it, *this) != 0) {
        ::std::ostringstream os;
        os << "get members fail! named group(?) " << name() << " is not group?";
        throw ::std::runtime_error(os.str());
    }
    it._next = it.next_i();
    return it;
}

ConstObjectIterator Group::members(void) const {
    ConstObjectIterator it;
    if (nm_group_members(&it._it, *this) != 0) {
        ::std::ostringstream os;
        os << "get members fail! named group(?) " << name() << " is not group?";
        throw ::std::runtime_error(os.str());
    }
    it._next = it.next_i();
    return it;
}

Object *
Group::findMember(cpe_hash_string_t name) {
    return Object::_cast(nm_group_find_member(*this, name));
}

Object const *
Group::findMember(cpe_hash_string_t name) const {
    return Object::_cast(nm_group_find_member(*this, name));
}

Object const & Group::member(cpe_hash_string_t name) const {
    Object const * r = findMember(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "named object " << cpe_hs_data(name)
           << " not exist in group " << this->name() << "!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Object & Group::member(cpe_hash_string_t name) {
    Object * r = findMember(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "named object " << cpe_hs_data(name)
           << " not exist in group " << this->name() << "!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Object *
Group::findMemberNc(const char * name) {
    return Object::_cast(nm_group_find_member_nc(*this, name));
}

Object const *
Group::findMemberNc(const char * name) const {
    return Object::_cast(nm_group_find_member_nc(*this, name));
}

Object const & Group::memberNc(const char * name) const {
    Object const * r = findMemberNc(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "named object " << name
           << " not exist in group " << this->name() << "!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Object & Group::memberNc(const char * name) {
    Object * r = findMemberNc(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "named object " << name
           << " not exist in group " << this->name() << "!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

}}
