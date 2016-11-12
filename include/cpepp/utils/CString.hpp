#ifndef CPEPP_UTILS_CSTRING_H
#define CPEPP_UTILS_CSTRING_H
#include <string>
#include "cpe/pal/pal_string.h"
#include "ClassCategory.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Utils {

class CString : public SimulateObject {
public:
    operator const char * (void) const { return (const char *)this; }
    operator char * (void) { return (char *)this; }
    bool isValid(void) const { void const * p = (void const *)this; return p != NULL; }
    const char * c_str(void) const { return (char*) this; }

    static CString const & _cast(const char * d) { 
        return * reinterpret_cast<CString const *>(d);
    }

    static CString & _cast(char * d) { 
        return * reinterpret_cast<CString *>(d);
    }
};

inline bool operator==(const char * l, CString const & r) { return strcmp(l, r) == 0; }
inline bool operator!=(const char * l, CString const & r) { return strcmp(l, r) != 0; }
inline bool operator<(const char * l, CString const & r) { return strcmp(l, r) < 0; }
inline bool operator<=(const char * l, CString const & r) { return strcmp(l, r) <= 0; }
inline bool operator>(const char * l, CString const & r) { return strcmp(l, r) > 0; }
inline bool operator>=(const char * l, CString const & r) { return strcmp(l, r) >= 0; }

inline bool operator==(CString const & l, const char * r) { return strcmp(l, r) == 0; }
inline bool operator!=(CString const & l, const char * r) { return strcmp(l, r) != 0; }
inline bool operator<(CString const & l, const char * r) { return strcmp(l, r) < 0; }
inline bool operator<=(CString const & l, const char * r) { return strcmp(l, r) <= 0; }
inline bool operator>(CString const & l, const char * r) { return strcmp(l, r) > 0; }
inline bool operator>=(CString const & l, const char * r) { return strcmp(l, r) >= 0; }

inline bool operator==(CString const & l, CString const & r) { return strcmp(l, r) == 0; }
inline bool operator!=(CString const & l, CString const & r) { return strcmp(l, r) != 0; }
inline bool operator<(CString const & l, CString const & r) { return strcmp(l, r) < 0; }
inline bool operator<=(CString const & l, CString const & r) { return strcmp(l, r) <= 0; }
inline bool operator>(CString const & l, CString const & r) { return strcmp(l, r) > 0; }
inline bool operator>=(CString const & l, CString const & r) { return strcmp(l, r) >= 0; }

inline bool operator==(::std::string const & l, CString const & r) { return strcmp(l.c_str(), r) == 0; }
inline bool operator!=(::std::string const & l, CString const & r) { return strcmp(l.c_str(), r) != 0; }
inline bool operator<(::std::string const & l, CString const & r) { return strcmp(l.c_str(), r) < 0; }
inline bool operator<=(::std::string const & l, CString const & r) { return strcmp(l.c_str(), r) <= 0; }
inline bool operator>(::std::string const & l, CString const & r) { return strcmp(l.c_str(), r) > 0; }
inline bool operator>=(::std::string const & l, CString const & r) { return strcmp(l.c_str(), r) >= 0; }

inline bool operator==(CString const & l, ::std::string const & r) { return strcmp(l, r.c_str()) == 0; }
inline bool operator!=(CString const & l, ::std::string const & r) { return strcmp(l, r.c_str()) != 0; }
inline bool operator<(CString const & l, ::std::string const & r) { return strcmp(l, r.c_str()) < 0; }
inline bool operator<=(CString const & l, ::std::string const & r) { return strcmp(l, r.c_str()) <= 0; }
inline bool operator>(CString const & l, ::std::string const & r) { return strcmp(l, r.c_str()) > 0; }
inline bool operator>=(CString const & l, ::std::string const & r) { return strcmp(l, r.c_str()) >= 0; }

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
