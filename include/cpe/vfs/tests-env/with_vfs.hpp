#ifndef CPE_VFS_TESTENV_WITH_CFG_H
#define CPE_VFS_TESTENV_WITH_CFG_H
#include "cpe/utils/error.h"
#include "cpe/utils/error_list.h"
#include "cpe/utils/tests-env/test-env.hpp"
#include "../vfs_types.h"

namespace cpe { namespace vfs { namespace testenv {

class with_vfs : public ::testenv::env<> {
public:
    with_vfs();
    
    vfs_mgr_t t_vfs(void);

    void SetUp();
    void TearDown();

private:
    vfs_mgr_t m_vfs;
};

}}}

#endif
