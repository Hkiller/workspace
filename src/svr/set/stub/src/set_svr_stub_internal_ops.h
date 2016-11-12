#ifndef SVR_SET_STUB_INTERNAL_OPS_H
#define SVR_SET_STUB_INTERNAL_OPS_H
#include "set_svr_stub_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*svr oprations*/
set_svr_stub_t
set_svr_stub_create(
    gd_app_context_t app, const char * name,
    uint16_t svr_type_id, mem_allocrator_t alloc, error_monitor_t em);
void set_svr_stub_free(set_svr_stub_t svr);

int set_svr_stub_outgoing_recv(dp_req_t req, void * ctx, error_monitor_t em);
ptr_int_t set_svr_stub_tick(void * ctx, ptr_int_t arg, float delta);
void set_svr_stub_set_chanel(set_svr_stub_t svr, set_chanel_t chanel);

int set_svr_stub_lock_pidfile(set_svr_stub_t svr, const char * pidfile);
int set_svr_stub_write_pidfile(set_svr_stub_t svr);

/*svr info operations*/
set_svr_svr_info_t
set_svr_svr_info_create(set_svr_stub_t svr, const char * svr_name, uint16_t svr_type);
void set_svr_svr_info_free(set_svr_stub_t stub, set_svr_svr_info_t svr_info);
void set_svr_svr_info_free_all(set_svr_stub_t svr);

uint32_t set_svr_svr_info_id_hash(set_svr_svr_info_t svr);
int set_svr_svr_info_id_eq(set_svr_svr_info_t l, set_svr_svr_info_t r);

/*cmd info operations*/
set_svr_cmd_info_t set_svr_cmd_info_create(set_svr_stub_t stub, set_svr_svr_info_t svr_info, LPDRMETAENTRY entry);
void set_svr_cmd_info_free_all(set_svr_stub_t stub, set_svr_svr_info_t svr_info);

uint32_t set_svr_cmd_info_hash(set_svr_cmd_info_t cmd_info);
int set_svr_cmd_info_eq(set_svr_cmd_info_t l, set_svr_cmd_info_t r);

/*app operations*/
int set_svr_app_run(gd_app_context_t ctx, void * user_ctx);

/*buff operations*/
set_svr_stub_buff_t set_svr_stub_buff_shm_attach(set_svr_stub_t stub, int shmid);
set_svr_stub_buff_t set_svr_stub_buff_shm_save(set_svr_stub_t stub, int shmid); /*只把shmid存下来 */
void set_svr_stub_buff_free_all(set_svr_stub_t stub);

#ifdef __cplusplus
}
#endif

#endif
