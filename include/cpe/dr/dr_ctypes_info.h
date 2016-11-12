#ifndef CPE_DR_CTYPES_H
#define CPE_DR_CTYPES_H
#include "cpe/dr/dr_define.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_external.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPE_DR_TYPE_UNKOWN      -1
#define CPE_DR_TYPE_MIN		    0x00
#define CPE_DR_TYPE_UNION		CPE_DR_TYPE_MIN	
#define CPE_DR_TYPE_STRUCT		0x01
#define	CPE_DR_TYPE_COMPOSITE	CPE_DR_TYPE_STRUCT
#define CPE_DR_TYPE_CHAR		0x02
#define CPE_DR_TYPE_UCHAR		0x03
#define CPE_DR_TYPE_INT8		0x04
#define CPE_DR_TYPE_INT16		0x05
#define CPE_DR_TYPE_UINT16		0x06
#define CPE_DR_TYPE_INT32		0x07
#define CPE_DR_TYPE_UINT32		0x08
/*#define CPE_DR_TYPE_LONG		0x09*/
/*#define CPE_DR_TYPE_ULONG		0x0a*/
#define CPE_DR_TYPE_INT64	    0x0b
#define CPE_DR_TYPE_UINT64	    0x0c
#define CPE_DR_TYPE_DATE		0x0d
#define CPE_DR_TYPE_TIME		0x0e
#define CPE_DR_TYPE_DATETIME	0x0f
#define CPE_DR_TYPE_MONEY		0x10
#define CPE_DR_TYPE_FLOAT		0x11
#define CPE_DR_TYPE_DOUBLE		0x12
#define CPE_DR_TYPE_IP			0x13
/*#define CPE_DR_TYPE_WCHAR		0x14*/
#define CPE_DR_TYPE_STRING		0x15
/*#define CPE_DR_TYPE_WSTRING	0x16*/
#define CPE_DR_TYPE_VOID		0x17
#define CPE_DR_TYPE_UINT8		0x18
#define CPE_DR_TYPE_MAX		    CPE_DR_TYPE_UINT8


CPE_DR_API const char * dr_type_name(int typeId);
CPE_DR_API int dr_type_size(int typeId);
CPE_DR_API int dr_type_id_from_name(const char * name);

CPE_DR_API char *dr_drip_to_ineta(dr_ip_t a_iTDRIp);
CPE_DR_API const char *dr_drip_ntop(dr_ip_t a_iTDRIp, char *a_pszDstBuff, int a_iBuffSize);
CPE_DR_API int dr_str_to_drdate(dr_date_t *a_piTdrDate, const char *a_pszDate);
CPE_DR_API char *dr_drdate_to_str(const dr_date_t *a_piTdrDate);
CPE_DR_API const char *dr_drdate_to_str_r(const dr_date_t *a_piTdrDate, 
										 char *a_pszDstBuf, int a_iBuffSize);

CPE_DR_API int dr_str_to_drtime(dr_time_t *a_piTdrTime, const char *a_pszTime);
CPE_DR_API char *dr_drtime_to_str(dr_time_t *a_piTdrTime);
CPE_DR_API char *dr_drtime_to_str_r(dr_time_t *a_piTdrTime, 
								   char *a_pszDstBuf, int a_iBuffSize);
CPE_DR_API int dr_str_to_drdatetime(dr_datetime_t *a_piTdrDateTime, const char *a_pszDateTime);
CPE_DR_API int dr_utctime_to_drdatetime(dr_datetime_t *a_piTdrDateTime, time_t a_tTimer);
CPE_DR_API int dr_drdatetime_to_utctime(time_t *a_ptTimer, dr_datetime_t a_iTdrDateTime);
CPE_DR_API char *dr_drdatetime_to_str(const dr_datetime_t *a_piTdrDateTime);
CPE_DR_API char *dr_drdatetime_to_str_r(const dr_datetime_t *a_piTdrDateTime, 
									   char *a_pszDstBuf, int a_iBuffSize);

CPE_DR_API int dr_compare_datetime(const dr_datetime_t *a_piTdrDateTime1, const dr_datetime_t *a_piTdrDateTime2);
CPE_DR_API int dr_compare_date(const dr_date_t *a_piTdrDate1, const dr_date_t *a_piTdrDate2);
CPE_DR_API int dr_compare_time(const dr_time_t *a_piTdrTime1, const dr_time_t *a_piTdrTime2);

#ifdef __cplusplus
}
#endif

#endif
