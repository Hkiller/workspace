#include "cpe/utils/tests-env/with_em.hpp"

namespace utils { namespace testenv {

void with_em::SetUp() {
    m_el = cpe_error_list_create(t_tmp_allocrator());
    cpe_error_monitor_init(&m_em, cpe_error_list_collect, m_el);
    cpe_error_monitor_node_init(&m_printer, cpe_error_log_to_consol, NULL);
}

void with_em::TearDown() {
    cpe_error_list_free(m_el);
    m_el = NULL;
}

error_monitor_t with_em::t_em() {
    return &m_em;
}

error_list_t with_em::t_elist() {
    return m_el;
}

void with_em::t_em_set_print() {
    cpe_error_monitor_add_node(&m_em, &m_printer);
}

int with_em::t_em_have_errno(int e) {
    return cpe_error_list_have_errno(m_el, e);
}

int with_em::t_em_error_count(void) {
    return cpe_error_list_error_count(m_el);
}

int with_em::t_em_no_error(void) {
    return t_em_error_count() == 0 ? 1 : 0;
}

void with_em::t_elist_clear(void) {
    cpe_error_list_clear(m_el);
}

}}

