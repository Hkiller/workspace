#ifndef CPE_PAL_PLATFORM_H
#define CPE_PAL_PLATFORM_H

#if defined _APPLE
#    include <ConditionalMacros.h>
#endif

//TODO
#if 0
#define CPE_BIG_ENDIAN
#else
#define CPE_LITTLE_ENDIAN
#endif

#ifndef __WORDSIZE
#if __SIZEOF_INT__ == 8
#    define __WORDSIZE (64)
#else
#    define __WORDSIZE (32)
#endif
#endif

#ifdef _MSC_VER
#  define CPE_DEFAULT_ALIGN (1)
#else
#  define CPE_DEFAULT_ALIGN (__WORDSIZE / 8)
#endif

#define CPE_PAL_CALC_ALIGN_8(__value) if ((__value) % 8) { (__value) = ((((__value) >> 3) + 1) << 3); }
#define CPE_PAL_CALC_ALIGN_4(__value) if ((__value) % 4) { (__value) = ((((__value) >> 2) + 1) << 2); }
#define CPE_PAL_CALC_ALIGN_2(__value) if ((__value) % 2) { (__value) = ((((__value) >> 1) + 1) << 1); }

#define CPE_PAL_CALC_ALIGN(__value, __align)    \
    switch(__align) {                           \
    case 2:                                     \
        CPE_PAL_CALC_ALIGN_2(__value);          \
        break;                                  \
    case 4:                                     \
        CPE_PAL_CALC_ALIGN_4(__value);          \
        break;                                  \
    case 8:                                     \
        CPE_PAL_CALC_ALIGN_8(__value);          \
        break;                                  \
    }


#if (__WORDSIZE == 64)
#define CPE_PAL_ALIGN_DFT CPE_PAL_CALC_ALIGN_8
#else
#define CPE_PAL_ALIGN_DFT CPE_PAL_CALC_ALIGN_4
#endif

#define CPE_COPY_ENDIAN64(outp, inp) do {           \
        const char * in = (const char *)(inp);    \
        char *out = (char *)(outp);               \
        out[0] = in[7];                         \
        out[1] = in[6];                         \
        out[2] = in[5];                         \
        out[3] = in[4];                         \
        out[4] = in[3];                         \
        out[5] = in[2];                         \
        out[6] = in[1];                         \
        out[7] = in[0];                         \
    } while(0)

#define CPE_COPY_ENDIAN32(outp, inp) do {       \
        const char * in = (const char *)(inp);  \
        char *out = (char *)(outp);             \
                                                \
        out[0] = in[3];                         \
        out[1] = in[2];                         \
        out[2] = in[1];                         \
        out[3] = in[0];                         \
    } while(0)

#define CPE_COPY_ENDIAN16(outp, inp) do {       \
        const char * in = (const char *)(inp);  \
        char *out = (char *)(outp);             \
                                                \
        out[0] = in[1];                         \
        out[1] = in[0];                         \
    } while(0)

#define CPE_SWAP_ENDIAN64(p) do {                 \
        char * inout = (char *)(p);               \
        char b;                                   \
        b = inout[0]; inout[0] = inout[7]; inout[7] = b;  \
        b = inout[1]; inout[1] = inout[6]; inout[6] = b;  \
        b = inout[2]; inout[2] = inout[5]; inout[5] = b;  \
        b = inout[3]; inout[3] = inout[4]; inout[4] = b;  \
    } while(0)

#define CPE_SWAP_ENDIAN32(p) do {                 \
        char * inout = (char *)(p);               \
        char b;                                   \
        b = inout[0]; inout[0] = inout[3]; inout[3] = b;  \
        b = inout[1]; inout[1] = inout[2]; inout[2] = b;  \
    } while(0)

#define CPE_SWAP_ENDIAN16(p) do {       \
        char * inout = (char *)(p);               \
        char b;                                   \
        b = inout[0]; inout[0] = inout[1]; inout[1] = b;  \
    } while(0)

#ifdef CPE_LITTLE_ENDIAN
#define CPE_COPY_HTON64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define CPE_COPY_NTOH64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define CPE_COPY_HTON32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define CPE_COPY_NTOH32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define CPE_COPY_HTON16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define CPE_COPY_NTOH16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define CPE_SWAP_HTON64(p) CPE_SWAP_ENDIAN64(p)
#define CPE_SWAP_NTOH64(p) CPE_SWAP_ENDIAN64(p)
#define CPE_SWAP_HTON32(p) CPE_SWAP_ENDIAN32(p)
#define CPE_SWAP_NTOH32(p) CPE_SWAP_ENDIAN32(p)
#define CPE_SWAP_HTON16(p) CPE_SWAP_ENDIAN16(p)
#define CPE_SWAP_NTOH16(p) CPE_SWAP_ENDIAN16(p)
#else
#define CPE_COPY_HTON64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define CPE_COPY_NTOH64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define CPE_COPY_HTON32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define CPE_COPY_NTOH32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define CPE_COPY_HTON16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define CPE_COPY_NTOH16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define CPE_SWAP_HTON64(p) do { } while(0)
#define CPE_SWAP_NTOH64(p) do { } while(0)
#define CPE_SWAP_HTON32(p) do { } while(0)
#define CPE_SWAP_NTOH32(p) do { } while(0)
#define CPE_SWAP_HTON16(p) do { } while(0)
#define CPE_SWAP_NTOH16(p) do { } while(0)
#endif

#ifdef __GNUC__
#define INLINE static inline
#else
#define INLINE static
#endif

#endif

