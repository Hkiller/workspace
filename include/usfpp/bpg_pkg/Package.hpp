#ifndef USFPP_BPG_PKG_PACKAGE_H
#define USFPP_BPG_PKG_PACKAGE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Bpg {

class PackageAppendInfo : public Cpe::Utils::SimulateObject {
public:
    operator bpg_pkg_append_info_t() const { return (bpg_pkg_append_info_t)this; }

    uint32_t id(void) const { return bpg_pkg_append_info_id(*this); }
    void * data(dp_req_t pkg) const { return bpg_pkg_append_data(pkg, *this); }
    uint32_t size(void) const { return bpg_pkg_append_info_size(*this); }
};

class Package : public Cpe::Utils::SimulateObject {
public:
    operator dp_req_t() const { return (dp_req_t)this; }
    operator bpg_pkg_t() const;

    PackageManager & mgr(void);
    PackageManager const & mgr(void) const;

    Gd::App::Application & app(void);
    Gd::App::Application const & app(void) const;

    void init(void) { bpg_pkg_init(*this); }
    void clearData(void) { bpg_pkg_clear(*this); }

    uint32_t cmd(void) const { return bpg_pkg_cmd(*this); }
    void setCmd(uint32_t cmd) { bpg_pkg_set_cmd(*this, cmd); }

    uint32_t sn(void) const { return bpg_pkg_sn(*this); }
    void setSn(uint32_t sn) { bpg_pkg_set_sn(*this, sn); }

    uint32_t errCode(void) const { return bpg_pkg_errno(*this); }
    void setErrCode(uint32_t en) { bpg_pkg_set_errno(*this, en); }

    uint64_t clientId(void) const { return bpg_pkg_client_id(*this); }
    void setClientId(uint64_t client_id) { bpg_pkg_set_client_id(*this, client_id); }

    Cpe::Dr::MetaLib const & dataMetaLib(void) const;

    /*main data and cmd write*/
    void setCmdAndData(Cpe::Dr::ConstData const & data);
    void setCmdAndData(Cpe::Dr::Data const & data);
    void setCmdAndData(Cpe::Dr::ConstData const & data, size_t size);
    void setCmdAndData(int cmd, const void * data, size_t data_size);
    void setCmdAndData(LPDRMETA meta, const void * data, size_t data_size);

    template<typename T>
    void setCmdAndData(int cmd, T const & data) { setCmdAndData(cmd, &data, sizeof(data)); }

    template<typename T>
    void setCmdAndData(T const & data) { setCmdAndData(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data)); }

    /*main data read*/
    Cpe::Dr::Meta const & mainDataMeta(void) const;
    Cpe::Dr::Meta const * tryGetMainDataMeta(void) const { return (Cpe::Dr::Meta const *)bpg_pkg_main_data_meta(*this, NULL); }

    size_t mainDataSize(void) const { return bpg_pkg_main_data_len(*this); }
    void const * mainData(void) const { return bpg_pkg_main_data(*this); }

    template<typename T>
    void mainData(T & buf) { mainData(&buf, Cpe::Dr::MetaTraits<T>::data_size(buf)); }
    void mainData(Cpe::Dr::Data & data);

    /*main data write*/
    void setMainData(void const * data, size_t size);

    template<typename T>
    void setMainData(T const & data) { setMainData(&data, Cpe::Dr::MetaTraits<T>::data_size(data)); }

    /*append data read*/
    int32_t appendInfoCount(void) const { return bpg_pkg_append_info_count(*this); }
    PackageAppendInfo const & appendInfoAt(int32_t pos) const;

    void appendData(int metaId, void * buf, size_t capacity, size_t * size = NULL) const;
    void appendData(const char * metaName, void * buf, size_t capacity, size_t * size = NULL) const;
    void appendData(LPDRMETA meta, void * buf, size_t capacity, size_t * size = NULL) const;

    bool tryGetAppendData(int metaId, void * buf, size_t capacity, size_t * size = NULL) const;
    bool tryGetAppendData(const char * metaName, void * buf, size_t capacity, size_t * size = NULL) const;
    bool tryGetAppendData(LPDRMETA meta, void * buf, size_t capacity, size_t * size = NULL) const;

    template<typename T>
    void appendData(int metaId, T & buf) { appendData(metaId, &buf, sizeof(buf)); }

    template<typename T>
    void appendData(const char * metaName, T & buf) { appendData(metaName, &buf, sizeof(buf)); }

    template<typename T>
    bool tryGetAppendData(int metaId, T & buf) { return tryGetAppendData(metaId, &buf, sizeof(buf)); }

    template<typename T>
    bool tryGetAppendData(const char * metaName, T & buf) { return tryGetAppendData(metaName, &buf, sizeof(buf)); }

    /*append data write*/
    void addAppendData(const char * metaName, void const * data, size_t size);
    void addAppendData(int metaid, void const * data, size_t size);
    void addAppendData(LPDRMETA meta, void const * data, size_t size);

    template<typename T>
    void addAppendData(const char * metaName, T const & data) {
        addAppendData(metaName, (void const *)&data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    template<typename T>
    void addAppendData(int metaId, T const & data) {
        addAppendData(metaId, (void const *)&data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    template<typename T>
    void addAppendData(T const & data) {
        addAppendData(Cpe::Dr::MetaTraits<T>::META, (void const *)&data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    /*other op*/
    const char * dump_data(mem_buffer_t buffer) const { return bpg_pkg_dump(*this, buffer); }

    static Package & _cast(dp_req_t req);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
