#ifndef CPEPP_DR_CTYPEUTILS_H
#define CPEPP_DR_CTYPEUTILS_H
#include "cpe/dr/dr_ctypes_op.h"
#include "System.hpp"

namespace Cpe { namespace Dr {

void dr_ctype_set_from_string_check_throw(void * output, int type, const char * input);

template<typename T>
struct CTypeTraits;

template<>
struct CTypeTraits<bool> {
    enum { id = CPE_DR_TYPE_UINT8 };
    typedef bool Type;
    static Type from_string(const char * data) {
        uint8_t tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp ? true : false;
    }
    
    static Type from_string(const char * data, Type dft) {
        uint8_t tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? (tmp ? true : false) : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { uint8_t tmp = data ? 1 : 0; return dr_ctype_to_string(buff, &tmp, id); }
};

template<>
struct CTypeTraits<uint8_t> {
    enum { id = CPE_DR_TYPE_UINT8 };
    typedef uint8_t Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<int8_t> {
    enum { id = CPE_DR_TYPE_INT8 };
    typedef int8_t Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<uint16_t> {
    enum { id = CPE_DR_TYPE_UINT16 };
    typedef uint16_t Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<int16_t> {
    enum { id = CPE_DR_TYPE_INT16 };
    typedef int16_t Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<uint32_t> {
    enum { id = CPE_DR_TYPE_UINT32 };
    typedef uint32_t Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<int32_t> {
    enum { id = CPE_DR_TYPE_INT32 };
    typedef int32_t Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<uint64_t> {
    enum { id = CPE_DR_TYPE_UINT64 };
    typedef uint64_t Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<int64_t> {
    enum { id = CPE_DR_TYPE_INT64 };
    typedef int64_t Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<float> {
    enum { id = CPE_DR_TYPE_FLOAT };
    typedef float Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<double> {
    enum { id = CPE_DR_TYPE_DOUBLE };
    typedef double Type;
    static Type from_string(const char * data) {
        Type tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        Type tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

template<>
struct CTypeTraits<const char *> {
    enum { id = CPE_DR_TYPE_STRING };
    typedef const char * Type;
    static Type from_string(const char * data) {
        return data;
    }
    
    static Type from_string(const char * data, Type dft) {
        return data;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return data; }
};

template<>
struct CTypeTraits<char *> {
    enum { id = CPE_DR_TYPE_STRING };
    typedef const char * Type;
    static Type from_string(const char * data) {
        return data;
    }
    
    static Type from_string(const char * data, Type dft) {
        return data;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return data; }
};

template<>
struct CTypeTraits<const unsigned char *> {
    enum { id = CPE_DR_TYPE_STRING };
    typedef const unsigned char * Type;
    static Type from_string(const char * data) {
        return (Type)data;
    }
    
    static Type from_string(const char * data, Type dft) {
        return (Type)data;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return (const char *)data; }
};

template<>
struct CTypeTraits<unsigned char *> {
    enum { id = CPE_DR_TYPE_STRING };
    typedef const unsigned char * Type;
    static Type from_string(const char * data) {
        return (Type)data;
    }
    
    static Type from_string(const char * data, Type dft) {
        return (Type)data;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return (const char *)data; }
};

#if defined _APPLE && __WORDSIZE == 64

template<>
struct CTypeTraits<size_t> {
    enum { id = CPE_DR_TYPE_UINT64 };
    typedef size_t Type;
    static Type from_string(const char * data) {
        uint64_t tmp; dr_ctype_set_from_string_check_throw(&tmp, id, data); return tmp;
    }
    
    static Type from_string(const char * data, Type dft) {
        uint64_t tmp; return dr_ctype_set_from_string(&tmp, id, data, NULL) == 0 ? tmp : dft;
    }
    
    static const char * to_string(mem_buffer_t buff, Type data) { return dr_ctype_to_string(buff, &data, id); }
};

#endif

}}

#endif

