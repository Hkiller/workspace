#ifndef CPE_DR_BSON_INTERNAL_OPS_H
#define CPE_DR_BSON_INTERNAL_OPS_H
#include "cpe/pal/pal_platform.h"
#include "dr_bson_internal_types.h"

#ifdef CPE_LITTLE_ENDIAN
#define BSON_COPY_HTON64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define BSON_COPY_NTOH64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define BSON_COPY_HTON32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define BSON_COPY_NTOH32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define BSON_COPY_HTON16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define BSON_COPY_NTOH16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#else
#define BSON_COPY_HTON64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define BSON_COPY_NTOH64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define BSON_COPY_HTON32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define BSON_COPY_NTOH32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define BSON_COPY_HTON16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define BSON_COPY_NTOH16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#endif

#endif
