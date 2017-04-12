#ifndef USF_MONGO_DRIVER_UTILS_H
#define USF_MONGO_DRIVER_UTILS_H
#include "mongo_driver_i.h"

/*字节序 */
#ifdef CPE_LITTLE_ENDIAN
#define MONGO_COPY_HTON64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define MONGO_COPY_NTOH64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define MONGO_COPY_HTON32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define MONGO_COPY_NTOH32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define MONGO_COPY_HTON16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define MONGO_COPY_NTOH16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define MONGO_SWAP_HTON64(p) do { } while(0)
#define MONGO_SWAP_NTOH64(p) do { } while(0)
#define MONGO_SWAP_HTON32(p) do { } while(0)
#define MONGO_SWAP_NTOH32(p) do { } while(0)
#define MONGO_SWAP_HTON16(p) do { } while(0)
#define MONGO_SWAP_NTOH16(p) do { } while(0)
#else
#define MONGO_COPY_HTON64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define MONGO_COPY_NTOH64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define MONGO_COPY_HTON32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define MONGO_COPY_NTOH32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define MONGO_COPY_HTON16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define MONGO_COPY_NTOH16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define MONGO_SWAP_HTON64(p) CPE_SWAP_ENDIAN64(p)
#define MONGO_SWAP_NTOH64(p) CPE_SWAP_ENDIAN64(p)
#define MONGO_SWAP_HTON32(p) CPE_SWAP_ENDIAN32(p)
#define MONGO_SWAP_NTOH32(p) CPE_SWAP_ENDIAN32(p)
#define MONGO_SWAP_HTON16(p) CPE_SWAP_ENDIAN16(p)
#define MONGO_SWAP_NTOH16(p) CPE_SWAP_ENDIAN16(p)
#endif


#endif
