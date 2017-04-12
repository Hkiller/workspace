#ifndef CPE_POM_TYPES_H
#define CPE_POM_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/range.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POM_MAX_TYPENAME_LEN 32
#define POM_MAX_TYPE_COUNT 255 /*typeid 0 is invalid!*/

#define POM_INVALID_CLASSID ((pom_class_id_t)0)
#define POM_INVALID_OID ((pom_oid_t)0)
#define POM_INVALID_BUFFER_ID ((pom_buffer_id_t)0)

typedef uint8_t pom_class_id_t;
typedef uint32_t pom_oid_t;

typedef struct pom_backend * pom_backend_t;
typedef struct pom_class * pom_class_t;
typedef struct pom_mgr * pom_mgr_t;
typedef struct pom_type * pom_type_t;
typedef struct pom_debuger * pom_debuger_t;

typedef ptr_int_t pom_buffer_id_t;

struct pom_buffer_it {
    size_t m_buf_size;
    struct cpe_urange_it m_urange_it;
    struct cpe_urange m_curent;
};

struct pom_buffer_id_it {
    struct cpe_urange_it m_urange_it;
    struct cpe_urange m_curent;
};

struct pom_backend {
    pom_buffer_id_t (*buf_alloc)(size_t size, void * context);
    void * (*buf_get)(pom_buffer_id_t id, void * context);
    void (*clear)(struct pom_buffer_id_it * buf_ids, void * context);
};

typedef struct pom_obj_it {
    void * (*next)(struct pom_obj_it * it);
    char m_data[16];
} * pom_obj_it_t;

#ifdef __cplusplus
}
#endif

#endif
