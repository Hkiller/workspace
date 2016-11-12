#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "rank_g_svr_db_ops.h"

/* int rank_g_svr_db_send_query(rank_g_svr_t svr, logic_require_t require, const char * id) { */
/*     int pkg_r; */
/*     mongo_pkg_t db_pkg; */

/*     db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db); */
/*     if (db_pkg == NULL) { */
/*         CPE_ERROR(svr->m_em, "%s: send_query: get db pkg fail!", rank_g_svr_name(svr)); */
/*         if (require) logic_require_set_error(require); */
/*         return -1; */
/*     } */

/*     mongo_pkg_init(db_pkg); */
/*     mongo_pkg_set_collection(db_pkg, "rank_g_use"); */
/*     mongo_pkg_set_op(db_pkg, mongo_db_op_query); */

/*     pkg_r = 0; */

/*     pkg_r |= mongo_pkg_doc_open(db_pkg); */
/*     pkg_r |= mongo_pkg_append_string(db_pkg, "_id", id); */
/*     pkg_r |= mongo_pkg_doc_close(db_pkg); */

/*     if (pkg_r) { */
/*         CPE_ERROR(svr->m_em, "%s: send_query: build query req fail!", rank_g_svr_name(svr)); */
/*         if (require) logic_require_set_error(require); */
/*         return -1; */
/*     } */

/*     if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 2, NULL, NULL, 0) != 0) { */
/*         CPE_ERROR(svr->m_em, "%s: send_query: send db request fail!", rank_g_svr_name(svr)); */
/*         if (require) logic_require_set_error(require); */
/*         return -1; */
/*     } */

/*     return 0; */
/* } */

int rank_g_svr_db_season_record_insert(rank_g_svr_t svr, logic_require_t require, void const * db_record) {
    mongo_pkg_t db_pkg;
    int pkg_r;
    SVR_RANK_G_SEASON_RECORD_COMMON const * record_common = db_record;
    
    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_insert: get db pkg fail!", rank_g_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "rank_g_season_record");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);
    mongo_pkg_update_set_flag(db_pkg, mongo_pro_flags_update_upsert);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "_id", (int64_t)record_common->_id);
    pkg_r |= mongo_pkg_doc_close(db_pkg);
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_season_record_meta, db_record, svr->m_season_record_size);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_insert: build insert req fail!", rank_g_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_insert: send db request fail!", rank_g_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}

int rank_g_svr_db_season_role_to_record_insert(rank_g_svr_t svr, logic_require_t require, SVR_RANK_G_SEASON_ROLE_TO_RANK const * role_to_rank) {
    mongo_pkg_t db_pkg;
    int pkg_r;
    
    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_insert: get db pkg fail!", rank_g_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "rank_g_season_role_to_rank");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);
    mongo_pkg_update_set_flag(db_pkg, mongo_pro_flags_update_upsert);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", role_to_rank->_id);
    pkg_r |= mongo_pkg_doc_close(db_pkg);
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_meta_season_role_to_rank, role_to_rank, sizeof(*role_to_rank));

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_insert: build insert req fail!", rank_g_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_insert: send db request fail!", rank_g_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}
