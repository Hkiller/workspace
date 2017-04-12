#ifndef CPE_UTILS_ERROR_H
#define CPE_UTILS_ERROR_H

#define CPE_SUCCESS				        0

#define CPE_ERR_MAKE(mod, e)    (0x80000000 | ((mod)<<24) | (e))
#define CPE_ERR_BASE(e)    ( (e) & 0xFFFFFF )

#define CPE_IS_ERROR(e)			( (e) & 0x80000000 )

#endif
