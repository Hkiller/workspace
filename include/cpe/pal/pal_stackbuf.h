#ifndef CPE_PAL_STACKBUF_H
#define CPE_PAL_STACKBUF_H

#ifndef CPE_MAX_STACK_BUF_LEN
#define CPE_MAX_STACK_BUF_LEN 128
#endif

#ifndef CPE_INTEGER_BUF_LEN
#define CPE_INTEGER_BUF_LEN 10
#endif

#if defined _MSC_VER
#define CPE_STACK_BUF_LEN_EX(__len, __max) (__max)
#else
#define CPE_STACK_BUF_LEN_EX(__len, __max) ((__len) > (__max) ? (__max) : (__len))
#endif

#define CPE_STACK_BUF_LEN(__len) CPE_STACK_BUF_LEN_EX(__len, CPE_MAX_STACK_BUF_LEN)

#endif
