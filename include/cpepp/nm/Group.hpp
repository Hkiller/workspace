#ifndef GDPP_NM_GROUP_H
#define GDPP_NM_GROUP_H
#include "Object.hpp"

namespace Cpe { namespace Nm {

class Group : public Object {
public:
    virtual ~Group() = 0;

    ObjectIterator members(void);
    ConstObjectIterator members(void) const;

    int memberCount(void) { return nm_group_member_count(*this); }
    void destoryMembers(void);
    void addMember(Object & object);

    Object * findMember(cpe_hash_string_t name);
    Object const * findMember(cpe_hash_string_t name) const;
    Object const & member(cpe_hash_string_t name) const;
    Object & member(cpe_hash_string_t name);

    Object * findMemberNc(const char * name);
    Object const * findMemberNc(const char * name) const;
    Object const & memberNc(const char * name) const;
    Object & memberNc(const char * name);

    void * operator new (size_t size, nm_mgr_t nmm, const char * name);
    void * operator new (size_t size, Manager & nmm, const char * name);
};

}}

#endif
