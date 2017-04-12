#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/vfs/tests-env/with_vfs.hpp"
#include "cpe/vfs/vfs_manage.h"

namespace cpe { namespace vfs { namespace testenv {

with_vfs::with_vfs(void)
    : m_vfs(NULL)
{
}

vfs_mgr_t with_vfs::t_vfs(void) {
    return m_vfs;
}

void with_vfs::SetUp() {
    error_monitor_t em = NULL;
    if (utils::testenv::with_em * e = tryEnvOf<utils::testenv::with_em>()) {
        em = e->t_em();
    }
    
    m_vfs = vfs_mgr_create(t_allocrator(), em);
    ASSERT_TRUE(m_vfs);
}

void with_vfs::TearDown() {
    if (m_vfs) {
        vfs_mgr_free(m_vfs);
        m_vfs = NULL;
    }
}

}}}
 
