#ifndef CPEPP_DR_ENTRY_H
#define CPEPP_DR_ENTRY_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Dr {

class Entry : public Cpe::Utils::SimulateObject {
public:
    operator LPDRMETAENTRY (void) const { return (LPDRMETAENTRY)this; }

    int version(void) const { return dr_entry_version(*this); }
    int id(void) const { return dr_entry_id(*this); }

    Utils::CString const & name(void) const { return Utils::CString::_cast(dr_entry_name(*this)); }
    Utils::CString const & cname(void) const { return Utils::CString::_cast(dr_entry_cname(*this)); }
    Utils::CString const & desc(void) const { return Utils::CString::_cast(dr_entry_desc(*this)); }

    Meta const & owner(void) const { return *(Meta const *)dr_entry_self_meta(*this); };

    const void * dftValue(void) const { return dr_entry_dft_value(*this); }

    int typeId(void) const { return dr_entry_type(*this); }
    Meta const * typeMeta(void) const { return (Meta const *)dr_entry_ref_meta(*this); }

    size_t size(void) const { return dr_entry_size(*this); }

    int arryCount(void) const { return dr_entry_array_count(*this); }
    Entry const * arrayCountRefer(void) const { return (Entry const *)dr_entry_array_refer_entry(*this); }

    Entry const * selector(void) const { return (Entry const *)dr_entry_select_entry(*this); }

    size_t startPos(int index = 0) const { return dr_entry_data_start_pos(*this, index); }
    int isKey(void) const { return dr_entry_is_key(*this) ? true : false; }

    static Entry const & _cast(LPDRMETAENTRY entry);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
