#include <limits>
#include <cassert>
#include "gdpp/app/Log.hpp"
#include "usf/logic/logic_manage.h"
#include "usfpp/logic_use/LogicOpDynData.hpp"

namespace Usf { namespace Logic {

void * LogicOpDynData::record(size_t i) {
    validate_data();

    void * r = logic_data_record_at(m_data, i);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(m_data)),
            ::std::runtime_error,
            "Usf::Logic::LogicOpDynData::recrd: get record at %d fail", (int)i);
    }

    return r;
}

void const * LogicOpDynData::record(size_t i) const {
    validate_data();

    void * r = logic_data_record_at(m_data, i);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(m_data)),
            ::std::runtime_error,
            "Usf::Logic::LogicOpDynData::recrd: get record at %d fail", (int)i);
    }

    return r;
}

void LogicOpDynData::recordRemove(size_t pos) {
    validate_data();

    if (logic_data_record_remove_by_pos(m_data, pos) != 0) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(m_data)),
            ::std::runtime_error,
            "Usf::Logic::LogicOpDynData::remove: remove at %d fail", (int)pos);
    }
}

void LogicOpDynData::recordRemove(void const * p) {
    validate_data();

    if (logic_data_record_remove_by_ins(m_data, (void*)p) != 0) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(m_data)),
            ::std::runtime_error,
            "Usf::Logic::LogicOpDynData::remove: remove %p fail", p);
    }
}

void LogicOpDynData::recordPop(void) {
    assert(recordCount() > 0);
    recordRemove(recordCount() - 1);
}

void * LogicOpDynData::recordAppend(void) {
    validate_data();

    void * r = logic_data_record_append_auto_inc(&m_data);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(m_data)),
            ::std::runtime_error,
            "Usf::Logic::LogicOpDynData::append: fail");
    }

    return r;
}

void LogicOpDynData::validate_data(void) const {
    if (m_data == NULL) {
        throw ::std::runtime_error("LogicOpDynData: data is invalid!"); 
    }
}

}}
