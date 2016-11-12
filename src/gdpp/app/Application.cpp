#include <limits>
#include "gd/app/app_basic.h"
#include "cpepp/nm/Object.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"

namespace Gd { namespace App {

Application &
Application::instance(void) {
    gd_app_context_t ins = gd_app_ins();
    if (ins == NULL) {
        throw ::std::runtime_error("Application have not been created!");
    }

    return *(Application*)ins;
}

Application &
Application::_cast(gd_app_context_t ctx) {
    if (ctx == NULL) {
        throw ::std::runtime_error("cast to Application fail: input ctx is NULL!");
    }
    return *(Application*)ctx;
}

Cpe::Tl::Manager & Application::tlManager(const char * name) {
    Cpe::Tl::Manager * r = findTlManager(name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(*this, ::std::runtime_error, "TlManager %s not exist!", name);
    }

    return *r;
}

Cpe::Tl::Manager const & Application::tlManager(const char * name) const {
    Cpe::Tl::Manager const * r = findTlManager(name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(*this, ::std::runtime_error, "TlManager %s not exist!", name);
    }

    return *r;
}

Cpe::Nm::Object const * Application::findObject(cpe_hash_string_t name) const {
    nm_node_t node = gd_app_context_find_node(*this, name);
    return node ? Cpe::Nm::Object::_cast(node) : NULL;
}

Cpe::Nm::Object * Application::findObject(cpe_hash_string_t name) {
    nm_node_t node = gd_app_context_find_node(*this, name);
    return node ? Cpe::Nm::Object::_cast(node) : NULL;
}

Cpe::Nm::Object const & Application::object(cpe_hash_string_t name) const {
    Cpe::Nm::Object const * r = findObject(name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(*this, ::std::runtime_error, "Nm::Object %s not exist!", cpe_hs_data(name));
    }
    return *r;
}

Cpe::Nm::Object & Application::object(cpe_hash_string_t name) {
    Cpe::Nm::Object * r = findObject(name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(*this, ::std::runtime_error, "Nm::Object %s not exist!", cpe_hs_data(name));
    }
    return *r;
}

Cpe::Nm::Object const * Application::findObjectNc(const char * name) const {
    nm_node_t node = gd_app_context_find_node_nc(*this, name);
    return node ? Cpe::Nm::Object::_cast(node) : NULL;
}

Cpe::Nm::Object * Application::findObjectNc(const char * name) {
    nm_node_t node = gd_app_context_find_node_nc(*this, name);
    return node ? Cpe::Nm::Object::_cast(node) : NULL;
}

Cpe::Nm::Object const & Application::objectNc(const char * name) const {
    Cpe::Nm::Object const * r = findObjectNc(name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(*this, ::std::runtime_error, "Nm::Object %s not exist!", name);
    }
    return *r;
}

Cpe::Nm::Object & Application::objectNc(const char * name) {
    Cpe::Nm::Object * r = findObjectNc(name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(*this, ::std::runtime_error, "Nm::Object %s not exist!", name);
    }
    return *r;
}

}}
