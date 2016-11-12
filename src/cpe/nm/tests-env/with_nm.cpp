#include <sstream>
#include "cpe/nm/tests-env/with_nm.hpp"

namespace gd { namespace nm { namespace testenv {

with_nm::with_nm()
    : m_nm(NULL)
{
}

void with_nm::SetUp() {
    m_nm = nm_mgr_create(t_allocrator());
}


void with_nm::TearDown() {
    nm_mgr_free(m_nm);
    m_nm = NULL;
}

nm_mgr_t with_nm::t_nm() {
    return m_nm;
}

nm_node_t
with_nm::t_nm_add_instance(const char * name, size_t capacity) {
    return nm_instance_create(t_nm(), name, capacity);
}

nm_node_t
with_nm::t_nm_add_group(const char * name, size_t capacity) {
    return nm_group_create(t_nm(), name, capacity);
}

nm_node_t with_nm::t_nm_find(const char * name) {
    return nm_mgr_find_node_nc(t_nm(), name);
}

int with_nm::t_nm_bind(const char * groupName, const char * instanceName) {
    return nm_group_add_member(
        t_nm_find(groupName),
        t_nm_find(instanceName));
}

::std::string
with_nm::t_nm_it_dump(nm_node_it_t it) {
    ::std::ostringstream os;
    while(nm_node_t node = nm_node_next(it)) {
        os << nm_node_name(node) << ":";
    }

    return os.str();
}

::std::string
with_nm::t_nm_node_groups(const char * nodeName) {
    struct nm_node_it it;
    if (nm_node_groups(&it, t_nm_find(nodeName)) == 0) {
        return t_nm_it_dump(&it);
    }
    else {
        return "init it error!";
    }
}

::std::string
with_nm::t_nm_group_members(const char * groupName) {
    struct nm_node_it it;
    if (nm_group_members(&it, t_nm_find(groupName)) == 0) {
        return t_nm_it_dump(&it);
    }
    else {
        return "init it error!";
    }
}

::std::string
with_nm::t_nm_nodes(void) {
    struct nm_node_it it;
    if (nm_mgr_nodes(&it, t_nm()) == 0) {
        return t_nm_it_dump(&it);
    }
    else {
        return "init it error!";
    }
}

nm_node_t
with_nm::t_nm_group_find(const char * groupName, const char * name) {
    nm_node_t group = t_nm_find(groupName);
    if (group == NULL) return NULL;

    return nm_group_find_member(group, cpe_hs_create(t_tmp_allocrator(), name));
}

}}}

