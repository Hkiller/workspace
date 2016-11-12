#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "usf/mongo_use/id_generator.h"
#include "mongo_use_internal_ops.h"

uint32_t mongo_id_info_hash(const struct mongo_id_info * id_info) {
    return cpe_hash_str(id_info->m_name, strlen(id_info->m_name));
}

int mongo_id_info_eq(const struct mongo_id_info * l, const struct mongo_id_info * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

int mongo_id_generator_regist_id(mongo_id_generator_t generator, const char * id_name, uint32_t id_start, uint32_t id_inc) {
    struct mongo_id_info * id_info;

    size_t name_len = strlen(id_name) + 1;

    id_info = (struct mongo_id_info *)mem_alloc(generator->m_alloc, sizeof(struct mongo_id_info) + name_len);
    if (id_info == NULL) return -1;

    memcpy(id_info + 1, id_name, name_len);

    id_info->m_name = (char *)(id_info + 1);
    id_info->m_next_id = 0;
    id_info->m_left_id_count = 0;
    id_info->m_id_start = id_start;
    id_info->m_id_inc = id_inc;

    id_info->m_processing_require = INVALID_LOGIC_REQUIRE_ID;

    id_info->m_waiting_queue =
        logic_require_queue_create(
            generator->m_gen.app, generator->m_alloc, generator->m_em, id_name,
            mongo_cli_proxy_logic_manage(generator->m_mongo_cli), 0);
    if (id_info->m_waiting_queue == NULL) {
        CPE_ERROR(generator->m_em, "%s.%s create: create require queue fail!", mongo_id_generator_name(generator), id_name);
        mem_free(generator->m_alloc, id_info);
        return -1;
    }

	cpe_hash_entry_init(&id_info->m_hh);
    if (cpe_hash_table_insert_unique(&generator->m_id_infos, id_info) != 0) {
        logic_require_queue_free(id_info->m_waiting_queue);
        mem_free(generator->m_alloc, id_info);
        return -1;
    }

    return 0;
}

void mongo_id_info_free(mongo_id_generator_t generator, struct mongo_id_info * id_info) {
    if (id_info->m_processing_require != INVALID_LOGIC_REQUIRE_ID) {
        logic_require_t require =
            logic_require_find(mongo_cli_proxy_logic_manage(generator->m_mongo_cli), id_info->m_processing_require);
        if (require) {
            CPE_INFO(
                generator->m_em, "%s: free id info %s: cancel require %d", 
                mongo_id_generator_name(generator), id_info->m_name, id_info->m_processing_require);
            logic_require_cancel(require);

            id_info->m_processing_require = INVALID_LOGIC_REQUIRE_ID;
        }
    }

    logic_require_queue_cancel_all(id_info->m_waiting_queue);
    logic_require_queue_free(id_info->m_waiting_queue);
    id_info->m_waiting_queue = NULL;


    cpe_hash_table_remove_by_ins(&generator->m_id_infos, id_info);
    mem_free(generator->m_alloc, id_info);
}

void mongo_id_info_free_all(mongo_id_generator_t generator) {
    struct cpe_hash_it id_info_it;
    struct mongo_id_info * id_info;

    cpe_hash_it_init(&id_info_it, &generator->m_id_infos);

    id_info = (struct mongo_id_info *)cpe_hash_it_next(&id_info_it);
    while (id_info) {
        struct mongo_id_info * next = (struct mongo_id_info *)cpe_hash_it_next(&id_info_it);

        mongo_id_info_free(generator, id_info);

        id_info = next;
    }
}

struct mongo_id_info *
mongo_id_info_find(mongo_id_generator_t generator, const char * id_name) {
    struct mongo_id_info key;
    key.m_name = id_name;

    return (struct mongo_id_info *)cpe_hash_table_find(&generator->m_id_infos, &key);
}

int mongo_id_info_have_waiting_require(mongo_id_generator_t generator, struct mongo_id_info * id_info) {
    logic_require_t require;

    if (id_info->m_processing_require == INVALID_LOGIC_REQUIRE_ID) return 0;

    require = logic_require_find(
        mongo_cli_proxy_logic_manage(generator->m_mongo_cli), id_info->m_processing_require);
    if (require == NULL || logic_require_state(require) != logic_require_state_waiting) return 0;

    return 1;
}
