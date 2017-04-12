#ifndef CPE_XCALC_TOKEN_I_H
#define CPE_XCALC_TOKEN_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/xcalc/xcalc_token.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(xtoken_list, xtoken) xtoken_list_t;

struct xtoken_data {
    union {
        int64_t _int;
        double _double;
    } num;
    struct {
        char * _string;
        char * _end;
    } str;
};

struct xtoken {
    uint32_t m_type;
    struct xtoken_data m_data;
    xtoken_t m_sub;
    TAILQ_ENTRY(xtoken) m_next;
};

xtoken_t xtoken_set_sub(xtoken_t token, xtoken_t sub);

/*type operations*/
#define xtoken_type_is_data(__token_type) (__token_type & XTOKEN_DATA_FLAG)
#define xtoken_type_is_sign(__token_type) (!xtoken_type_is_data(__token_type))
#define xtoken_type_index(__token_type) ((__token_type & 0xFF))

#define xtoken_is_data(__token) xtoken_type_is_data((__token)->m_type)
#define xtoken_is_sign(__token) xtoken_type_is_sign((__token)->m_type)

#define xtoken_set_type(__token, __type) ((__token)->m_type = (uint32_t)__type)
#define xtoken_get_type(__token) ((__token)->m_type)
const char * xtoken_type_name(uint32_t token_type);

/*int operations*/
#define xtoken_get_int(__token) ((__token)->m_data.num._int)
#define xtoken_set_int(__token, __val) ((__token)->m_data.num._int = (int64_t)__val)

/*double operations*/
#define xtoken_get_double(__token) ((__token)->m_data.num._double)
#define xtoken_set_double(__token, __val) ((__token)->m_data.num._double = (double)(__val))
double xtoken_get_double_2(xtoken_t token);

void xtoken_set_str(xtoken_t token, char * begin, char * end);

#define XTOKEN_DATA_FLAG 0x00008000u
#define XTOKEN_ADD 0x0u
#define XTOKEN_SUB 0x1u
#define XTOKEN_MUL 0x2u
#define XTOKEN_DIV 0x3u
#define XTOKEN_EQU 0x4u
#define XTOKEN_NE  0x5U
#define XTOKEN_BG  0x6U
#define XTOKEN_BE  0x7U
#define XTOKEN_LT  0x8u
#define XTOKEN_LE  0x9u
#define XTOKEN_AND 0xAu
#define XTOKEN_OR  0xBu
#define XTOKEN_NOT 0xCu
#define XTOKEN_LEFT_BRACKET 0xDu
#define XTOKEN_RIGHT_BRACKET 0xEu
#define XTOKEN_COLON 0xFu
#define XTOKEN_QES 0x10u
#define XTOKEN_COMMA 0x11u
#define XTOKEN_FUNC 0x12u
#define XTOKEN_END 0x13u
#define XTOKEN_VAL (XTOKEN_DATA_FLAG | 0x14u)
#define XTOKEN_NUM_INT  (XTOKEN_DATA_FLAG | 0x15u)
#define XTOKEN_NUM_FLOAT  (XTOKEN_DATA_FLAG | 0x16u)
#define XTOKEN_STRING  (XTOKEN_DATA_FLAG | 0x17u)
#define XTOKEN_ERROR 0x18u
#define XTOKEN_NUMBER 0x19u

#ifdef __cplusplus
}
#endif

#endif
