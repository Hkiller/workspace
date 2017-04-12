#include "assert.h"
#include "cpe/pal/pal_stdlib.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_manage.h"
#include "usf/bpg_net/bpg_net_client.h"
#include "bpg_net_internal_ops.h"

static void bpg_net_client_check_requires(bpg_net_client_t client) {
    uint32_t i;
    int remove_count = 0;

    for(i = 0; i < client->m_runing_require_count; ) {
        logic_require_t require = logic_require_find(client->m_logic_mgr, client->m_runing_requires[i]);
        if (require == NULL) {
            if (i + 1 < client->m_runing_require_count) {
                memmove(
                    client->m_runing_requires + i,
                    client->m_runing_requires + i + 1,
                    sizeof(logic_require_id_t) * (client->m_runing_require_count - i - 1));
            }
            --client->m_runing_require_count;
            ++remove_count;
        }
        else {
            ++i;
        }
    }

    if (client->m_debug) {
        CPE_INFO(
            client->m_em, "%s: notify_check_requires: remove %d, remain count %d!",
            bpg_net_client_name(client), remove_count, client->m_runing_require_count);
    }
}

int bpg_net_client_save_require_id(bpg_net_client_t client, logic_require_id_t id) {
    int i;

    if (client->m_runing_require_op_count >= client->m_runing_require_check_span) {
        bpg_net_client_check_requires(client);
        client->m_runing_require_op_count = 0;
    }

    if (client->m_runing_require_count >= client->m_runing_require_capacity) {
        uint32_t new_capacity;
        logic_require_id_t * new_buf;
        new_capacity = 
            client->m_runing_require_capacity < 128 ? 128 : client->m_runing_require_capacity * 2;
        new_buf = mem_alloc(client->m_alloc, sizeof(logic_require_id_t) * new_capacity);
        if (new_buf == NULL) return -1;

        if (client->m_runing_requires) {
            memcpy(new_buf, client->m_runing_requires, sizeof(logic_require_id_t) * client->m_runing_require_count);
            mem_free(client->m_alloc, client->m_runing_requires);
        }

        client->m_runing_requires = new_buf;
        client->m_runing_require_capacity = new_capacity;
    }

    assert(client->m_runing_requires);
    assert(client->m_runing_require_count < client->m_runing_require_capacity);

    client->m_runing_requires[client->m_runing_require_count] = id;
    ++client->m_runing_require_count;
    for(i = client->m_runing_require_count - 1; i > 0; --i) {
        logic_require_id_t buf;
        if (client->m_runing_requires[i] >= client->m_runing_requires[i - 1]) break;

        buf = client->m_runing_requires[i];
        client->m_runing_requires[i] = client->m_runing_requires[i - 1];
        client->m_runing_requires[i - 1] = buf;
    }

    ++client->m_runing_require_op_count;

    if (client->m_debug >= 2) {
        CPE_INFO(
            client->m_em, "%s: bpg_net_client_remove_require_id: add require %d at %d, count=%d, op-count=%d!",
            bpg_net_client_name(client), id, i, client->m_runing_require_count, client->m_runing_require_op_count);
    }

    return 0;
}

int bpg_net_client_require_id_cmp(const void * l, const void * r) {
    logic_require_id_t l_id = *((const logic_require_id_t *)l);
    logic_require_id_t r_id = *((const logic_require_id_t *)r);

    return l_id < r_id ? -1
        : l_id == r_id ? 0
        : 1;
}

int bpg_net_client_remove_require_id(bpg_net_client_t client, logic_require_id_t id) {
    logic_require_id_t * found;
    int found_pos;

    if (client->m_runing_require_count == 0) {
        if (client->m_debug >= 2) {
            CPE_INFO(
                client->m_em, "%s: bpg_net_client_remove_require_id: remove require %d fail, no any require!",
                bpg_net_client_name(client), id);
        }
        return -1;
    }

    assert(client->m_runing_requires);

    found =
        (logic_require_id_t *)bsearch(
            &id,
            client->m_runing_requires,
            client->m_runing_require_count,
            sizeof(id),
            bpg_net_client_require_id_cmp);
    if (!found) {
        if (client->m_debug >= 2) {
            CPE_INFO(
                client->m_em, "%s: bpg_net_client_remove_require_id: remove require %d fail, not found!",
                bpg_net_client_name(client), id);
        }
        return -1;
    }

    found_pos = found - client->m_runing_requires;
    assert(found_pos >= 0 && (uint32_t)found_pos < client->m_runing_require_count);

    if ((uint32_t)(found_pos + 1) < client->m_runing_require_count) {
        memmove(found, found + 1, sizeof(logic_require_id_t) * (client->m_runing_require_count - found_pos - 1));
    }

    --client->m_runing_require_count;

    if (client->m_debug >= 2) {
        CPE_INFO(
            client->m_em, "%s: bpg_net_client_remove_require_id: remove require %d at %d, left-count=%d!",
            bpg_net_client_name(client), id, found_pos, client->m_runing_require_count);
    }

    return 0;
}

void bpg_net_client_notify_all_require_disconnect(bpg_net_client_t client) {
    uint32_t i;
    int notified_count = 0;

    for(i = 0; i < client->m_runing_require_count; ++i) {
        logic_require_t require = logic_require_find(client->m_logic_mgr, client->m_runing_requires[i]);
        if (require) {
            ++notified_count;
            logic_require_set_error(require);
        }
    }

    client->m_runing_require_count = 0;

    if (client->m_debug) {
        CPE_INFO(
            client->m_em, "%s: notify_all_rquire_disconect: processed %d requires!",
            bpg_net_client_name(client), notified_count);
    }
}


