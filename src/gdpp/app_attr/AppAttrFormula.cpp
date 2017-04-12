#include "gdpp/app/Log.hpp"
#include "gd/app_attr/app_attr_module.h"
#include "gd/app_attr/app_attr_request.h"
#include "gdpp/app_attr/AppAttrFormula.hpp"

namespace Gd { namespace AppAttr {

AppAttrFormula & AppAttrFormula::calc(dr_data_source_t addition_arg) {
    if (app_attr_formula_calc(*this, addition_arg) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s calc fail!", name());
    }
    
    return *this;
}

int8_t AppAttrFormula::asInt8(void) {
    int8_t r;

    if (app_attr_formula_try_to_int8(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to int8 fail!", name());
    }
    
    return r;
}

uint8_t AppAttrFormula::asUInt8(void) {
    uint8_t r;

    if (app_attr_formula_try_to_uint8(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to uint8 fail!", name());
    }
    
    return r;
}

int16_t AppAttrFormula::asInt16(void) {
    int16_t r;

    if (app_attr_formula_try_to_int16(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to int16 fail!", name());
    }
    
    return r;
}

uint16_t AppAttrFormula::asUInt16(void) {
    uint16_t r;

    if (app_attr_formula_try_to_uint16(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to uint16 fail!", name());
    }
    
    return r;
}

int32_t AppAttrFormula::asInt32(void) {
    int32_t r;

    if (app_attr_formula_try_to_int32(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to int32 fail!", name());
    }
    
    return r;
}

uint32_t AppAttrFormula::asUInt32(void) {
    uint32_t r;

    if (app_attr_formula_try_to_uint32(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to uint32 fail!", name());
    }
    
    return r;
}

int64_t AppAttrFormula::asInt64(void) {
    int64_t r;

    if (app_attr_formula_try_to_int64(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to int64 fail!", name());
    }
    
    return r;
}

uint64_t AppAttrFormula::asUInt64(void) {
    uint64_t r;

    if (app_attr_formula_try_to_uint64(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to uint64 fail!", name());
    }
    
    return r;
}

float AppAttrFormula::asFloat(void) {
    float r;

    if (app_attr_formula_try_to_float(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to float fail!", name());
    }
    
    return r;
}

double AppAttrFormula::asDouble(void) {
    double r;

    if (app_attr_formula_try_to_double(*this, &r) != 0) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to double fail!", name());
    }
    
    return r;
}

const char * AppAttrFormula::asString(mem_buffer_t buffer) {
    const char * r;

    r = app_attr_formula_try_to_string(*this, buffer);
    if (r == NULL) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(app_attr_formula_request(*this)));
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "AppAttrFormula: formula %s convert to string fail!", name());
    }

    return r;
}

AppAttrFormula & AppAttrFormula::_cast(app_attr_formula_t formula) {
    if (formula == NULL) {
        throw ::std::runtime_error("Gd::Evt::AppAttrFormula::_cast: request is NULL!");
    }

    return *(AppAttrFormula*)formula;
}

}}


