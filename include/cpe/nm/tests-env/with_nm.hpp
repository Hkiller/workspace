#ifndef CPE_NM_TESTENV_WITHDP_H
#define CPE_NM_TESTENV_WITHDP_H
#include <string>
#include "cpe/utils/tests-env/test-env.hpp"
#include "../nm.h"

namespace gd { namespace nm { namespace testenv {

class with_nm : public ::testenv::env<> {
public:
    with_nm();

    void SetUp();
    void TearDown();

    nm_mgr_t t_nm();

    nm_node_t t_nm_add_instance(const char * name, size_t capacity);
    nm_node_t t_nm_add_group(const char * name, size_t capacity);
    int t_nm_bind(const char * groupName, const char * instanceName);
    nm_node_t t_nm_find(const char * name);

    ::std::string t_nm_it_dump(nm_node_it_t it);
    ::std::string t_nm_node_groups(const char * nodeName);
    ::std::string t_nm_nodes(void);
    ::std::string t_nm_group_members(const char * groupName);
    nm_node_t t_nm_group_find(const char * groupName, const char * name);
private:
    nm_mgr_t m_nm;
};

}}}

#endif

