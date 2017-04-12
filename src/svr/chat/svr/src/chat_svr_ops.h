#ifndef SVR_CHAT_SVR_OPS_H
#define SVR_CHAT_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "chat_svr_types.h"
#include "protocol/svr/chat/svr_chat_pro.h"

/*operations of chat_svr */
chat_svr_t
chat_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void chat_svr_free(chat_svr_t svr);

int chat_svr_gen_id(chat_svr_t svr, uint64_t * chat_id);
uint32_t chat_svr_cur_time(chat_svr_t svr);

chat_svr_t chat_svr_find(gd_app_context_t app, cpe_hash_string_t name);
chat_svr_t chat_svr_find_nc(gd_app_context_t app, const char * name);
const char * chat_svr_name(chat_svr_t svr);

int chat_svr_set_send_to(chat_svr_t svr, const char * send_to);
int chat_svr_set_recv_at(chat_svr_t svr, const char * name);
int chat_svr_set_check_span(chat_svr_t svr, uint32_t span_ms);

/*chanel data*/
chat_svr_chanel_t
chat_svr_chanel_create(
    chat_svr_t svr, SVR_CHAT_CHANEL_INFO const * chanek_info, uint64_t chanel_id);
void chat_svr_chanel_free(chat_svr_chanel_t chat);
void chat_svr_chanel_free_all(chat_svr_t svr);
chat_svr_chanel_t chat_svr_chanel_find(chat_svr_t svr, uint16_t chanel_type, uint64_t chanel_id);

uint64_t chat_svr_chanel_count(chat_svr_t svr);
SVR_CHAT_MSG * chat_svr_chanel_append_msg(chat_svr_chanel_t chanel);
SVR_CHAT_MSG * chat_svr_chanel_msg(chat_svr_chanel_t chanel, uint32_t pos);
uint32_t chat_svr_chanel_msg_count(chat_svr_chanel_t chanel);

uint32_t chat_svr_chanel_hash(chat_svr_chanel_t room);
int chat_svr_chanel_eq(chat_svr_chanel_t l, chat_svr_chanel_t r);

/*chat meta operations*/
int chat_svr_meta_chanel_load(chat_svr_t svr, cfg_t cfg);
SVR_CHAT_CHANEL_INFO const * chat_svr_meta_chanel_find(chat_svr_t svr, uint16_t chat_chat_type);

/*响应操作 */
typedef void (*chat_svr_op_t)(chat_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);
void chat_svr_op_query(chat_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);
void chat_svr_op_send(chat_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);

/*构造响应 */
void chat_svr_send_error_response(chat_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body, int err);

#endif
