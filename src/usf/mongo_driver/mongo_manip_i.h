#ifndef USF_MONGO_MANIP_I_H
#define USF_MONGO_MANIP_I_H
#include "mongo_driver_i.h"
#include "mongo_pkg_i.h"

/*utils*/
const char * mongo_pkg_build_authenticate_pass_digest(mongo_driver_t driver);

/**/
mongo_pkg_t mongo_pkg_build_check_is_master(mongo_driver_t driver);
mongo_pkg_t mongo_pkg_build_check_readable(mongo_driver_t driver);

/*authenticate(cr)*/
mongo_pkg_t mongo_pkg_build_cr_getnonce(mongo_driver_t driver);
mongo_pkg_t mongo_pkg_build_cr_authenticate(mongo_driver_t driver, const char * nonce);

/*authenticate(scram)*/
mongo_pkg_t mongo_pkg_build_scram_start(mongo_connection_t connection);
mongo_pkg_t mongo_pkg_build_scram_step2(mongo_connection_t connection, int32_t conv_id, char * payload);
mongo_pkg_t mongo_pkg_build_scram_step3(mongo_connection_t connection, int32_t conv_id, char * payload);

#endif
