#include "cpe/cfg/cfg_read.h"
#include "cpepp/dr/MetaLib.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Log.hpp"
#include "usfpp/bpg_pkg/Package.hpp"
#include "usfpp/bpg_use/SendPoint.hpp"

namespace Usf { namespace Bpg {

SendPoint &
SendPoint::instance(gd_app_context_t app, const char * name) {
    bpg_use_sp_t sp = bpg_use_sp_find_nc(app, name);
    if (sp == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "bpg_use_sp %s not exist!", name);
    }

    return *(SendPoint*)sp;
}

PackageManager const &
SendPoint::pkgManager(void) const {
    bpg_pkg_manage_t pkg_manage = bpg_use_sp_pkg_manage(*this);
    if (pkg_manage == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: have no pkg_manage!", name());
    }

    return *(PackageManager*)pkg_manage;
}

void SendPoint::send(Usf::Bpg::Package & pkg) {
    if (bpg_use_sp_send(*this, pkg) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: send pkg fail!", name());
    }
}

void SendPoint::send(Cpe::Dr::Data const & data) {
    Usf::Bpg::Package & pkg = pkgBuf(data.capacity()) ;
    pkg.clearData();
    pkg.setErrCode(0);
    pkg.setCmdAndData(data);
    send(pkg);
}

void SendPoint::send(LPDRMETA meta, void const * data, size_t size) {
    if (size == 0) size = dr_meta_size(meta);

    Usf::Bpg::Package & pkg = pkgBuf(size) ;
    pkg.clearData();
    pkg.setErrCode(0);
    pkg.setCmdAndData(meta, data, size);
    send(pkg);
}

Cpe::Dr::MetaLib const & SendPoint::metaLib(void) const {
    LPDRMETALIB metalib = bpg_use_sp_metalib(*this);
    if (metalib == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: meta lib not exist!", name());
    }

    return Cpe::Dr::MetaLib::_cast(metalib);
}

Cpe::Dr::Meta const & SendPoint::meta(const char * metaName) const {
    LPDRMETA meta = bpg_use_sp_meta(*this, metaName);
    if (meta == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: meta %s not exist!", name(), metaName);
    }

    return Cpe::Dr::Meta::_cast(meta);
}

Usf::Bpg::Package & SendPoint::pkgBuf(size_t capacity) {
    dp_req_t pkg_buf = bpg_use_sp_pkg_buf(*this, capacity);
    if (pkg_buf == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: pkg-buf is NULL!", name());
    }

    return Usf::Bpg::Package::_cast(pkg_buf);
}

Cpe::Dr::Data SendPoint::dataBuf(LPDRMETA meta, size_t capacity) {
    if (capacity == 0) capacity = dr_meta_size(meta);

    void * buf = bpg_use_sp_data_buf(*this, capacity);
    if (buf == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: data-buf is NULL!", name());
    }

    return Cpe::Dr::Data(buf, meta, capacity);
}

Cpe::Dr::Data SendPoint::dataBuf(size_t capacity) {
    void * buf = bpg_use_sp_data_buf(*this, capacity);
    if (buf == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: data-buf is NULL!", name());
    }

    return Cpe::Dr::Data(buf, capacity);
}

Cpe::Dr::Data SendPoint::dataDynBuf(LPDRMETA meta, size_t record_count) {
    ssize_t capacity = dr_meta_calc_dyn_size(meta, record_count);
    if (capacity < 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: calc dyn size of %s fail!", name(), dr_meta_name(meta));
    }

    return dataBuf(meta, capacity);
}

}}
