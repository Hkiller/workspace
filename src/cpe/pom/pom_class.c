#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/bitarry.h"
#include "cpe/utils/range_bitarry.h"
#include "cpe/pom/pom_error.h"
#include "pom_internal_ops.h"

uint32_t pom_class_hash_fun(struct pom_class * theClass) {
    return cpe_hs_value(theClass->m_name);
}

int pom_class_cmp_fun(struct pom_class * l, struct pom_class * r) {
    return cpe_hs_cmp(l->m_name, r->m_name) == 0;
}

static int pom_class_init(struct pom_class * theClass, mem_allocrator_t alloc) {
    theClass->m_name = (cpe_hash_string_t)theClass->m_name_buf;

    cpe_hash_entry_init(&theClass->m_hh);

    if (cpe_urange_mgr_init(&theClass->m_urange_alloc, alloc) != 0) return -1;

    return 0;
}

static void pom_class_fini(struct pom_class * theClass) {
    cpe_urange_mgr_fini(&theClass->m_urange_alloc);
}

static void pom_class_mgr_classes_fini(struct pom_class_mgr * classMgr, int endPos) {
    int pos;
    for(pos = endPos - 1; pos >= 0; --pos) {
        pom_class_fini(&classMgr->m_classes[pos]);
    }
}

int pom_class_mgr_init(struct pom_class_mgr * classMgr, mem_allocrator_t alloc) {
    int i;

    assert(classMgr);

    bzero(classMgr, sizeof(struct pom_class_mgr));

    for(i = 0; i < POM_CLASS_BUF_LEN; ++i) {
        if(pom_class_init(&classMgr->m_classes[i], alloc) != 0) {
            pom_class_mgr_classes_fini(classMgr, i);
            return -1;
        }
    }

    if (cpe_hash_table_init(
            &classMgr->m_classNameIdx,
            alloc,
            (cpe_hash_fun_t)pom_class_hash_fun,
            (cpe_hash_eq_t)pom_class_cmp_fun,
            CPE_HASH_OBJ2ENTRY(pom_class, m_hh),
            -1) != 0)
    {
        pom_class_mgr_classes_fini(classMgr, i);
        return -1;
    }

    return 0;
}

void pom_class_mgr_fini(struct pom_class_mgr * classMgr) {
    assert(classMgr);

    pom_class_mgr_classes_fini(classMgr, POM_CLASS_BUF_LEN);
    cpe_hash_table_fini(&classMgr->m_classNameIdx);
}

pom_class_id_t
pom_class_add(
    struct pom_class_mgr * classMgr,
    const char * className,
    size_t object_size,
    size_t page_size,
    size_t align,
    error_monitor_t em)
{
    int i;

    assert(classMgr);
    assert(className);

    for(i = 1; i < (int)POM_CLASS_BUF_LEN; ++i) {
        if (classMgr->m_classes[i].m_id == POM_INVALID_CLASSID) {
            break;
        }
    }

    if (i >= POM_CLASS_BUF_LEN) {
        CPE_ERROR_EX(em, pom_class_overflow, "too many class!");
        return POM_INVALID_CLASSID;
    }

    return 
        pom_class_add_with_id(
            classMgr,
            (pom_class_id_t)i,
            className,
            object_size,
            page_size,
            align,
            em) == 0
        ? (pom_class_id_t)i
        : POM_INVALID_CLASSID;
}

int pom_class_add_with_id(
    struct pom_class_mgr * classMgr,
    pom_class_id_t classId,
    const char * className,
    size_t object_size,
    size_t page_size,
    size_t align,
    error_monitor_t em)
{
    struct pom_class * theClass;

    assert(classMgr);
    assert(className);

    if (align != 1 && align != 2 && align != 4 && align != 8) {
        CPE_ERROR_EX(em, pom_invalid_align, "invalid align "  FMT_SIZE_T "!", align);
        return -1;
    }

    if (object_size % align) {
        object_size = ((object_size / align) + 1) * align;
    }

    if (classId <= 0) {
        CPE_ERROR_EX(em, pom_class_overflow, "invalid class id!");
        return -1;
    }

    if (((int)classId - 1) >= (int)POM_CLASS_BUF_LEN - 1) {
        CPE_ERROR_EX(em, pom_class_overflow, "class id overflow!");
        return -1;
    }

    if (strlen(className) > POM_MAX_TYPENAME_LEN) {
        CPE_ERROR_EX(em, pom_class_name_too_long, "class name too long!");
        return -1;
    }

    theClass = &classMgr->m_classes[classId];

    if (page_size > 0x1FFFFFFF) {
        CPE_ERROR_EX(
            em, pom_page_size_too_big,
            "page size("  FMT_SIZE_T ") is bigger then %d!", page_size, 0x1FFFFFFF);
        return -1;
    }

    assert(theClass->m_name); /*set in init, point to m_name_buf*/
    cpe_hs_init(theClass->m_name, cpe_hs_len_to_binary_len(POM_MAX_TYPENAME_LEN), className);

    theClass->m_object_size = object_size;
    theClass->m_page_size = page_size;

    theClass->m_object_per_page = ((page_size - sizeof(struct pom_data_page_head)) << 3) / ((object_size << 3) + 1);
    if (theClass->m_object_per_page < 10) {
        CPE_ERROR_EX(
            em, pom_page_size_too_small,
            "page size("  FMT_SIZE_T ") is too small, only can contain "  FMT_SIZE_T " object(s)!",
            page_size, theClass->m_object_per_page);
        return -1;
    }

    theClass->m_alloc_buf_capacity = cpe_ba_bytes_from_bits(theClass->m_object_per_page);
    theClass->m_object_buf_begin_in_page = (sizeof(struct pom_data_page_head) + theClass->m_alloc_buf_capacity);

    if (theClass->m_object_buf_begin_in_page % align) {
        theClass->m_object_buf_begin_in_page = ((theClass->m_object_buf_begin_in_page / align) + 1) * align;
        theClass->m_object_per_page = (page_size - theClass->m_object_buf_begin_in_page) / object_size;
        theClass->m_alloc_buf_capacity = cpe_ba_bytes_from_bits(theClass->m_object_per_page);
    }

    assert((theClass->m_object_per_page * theClass->m_object_size)
           <= (theClass->m_page_size - theClass->m_object_buf_begin_in_page));

    if (cpe_hash_table_insert_unique(&classMgr->m_classNameIdx, theClass) != 0) {
        CPE_ERROR_EX(
            em, pom_class_name_duplicate,
            "class %s name duplicate!", className);
        return -1;
    }

    /*last operation, set id*/
    theClass->m_id = classId;
    return 0;
}

struct pom_class *
pom_class_get(struct pom_class_mgr * classMgr, pom_class_id_t classId) {
    struct pom_class * r;

    assert(classMgr);

    r = &classMgr->m_classes[classId];

    return r->m_id == POM_INVALID_CLASSID
        ? NULL
        : r;
}

struct pom_class *
pom_class_find(struct pom_class_mgr * classMgr, cpe_hash_string_t className) {
    struct pom_class key;

    assert(classMgr);
    assert(className);

    key.m_name = className;

    return (struct pom_class *)cpe_hash_table_find(&classMgr->m_classNameIdx, &key);
}

static int pom_class_reserve_page_array_slot(struct pom_class * theClass, error_monitor_t em) {
    if (theClass->m_page_array_size >= theClass->m_page_array_capacity) {
        size_t new_page_array_capacity = theClass->m_page_array_capacity + 1024;
        void * new_page_array = mem_alloc(theClass->m_alloc, pom_class_page_buf_len(new_page_array_capacity));
        if (new_page_array == NULL) {
            CPE_ERROR_EX(em, pom_no_memory, "alloc for buffer list fail!");
            return -1;
        }

        if (theClass->m_page_array_size > 0) {
            memcpy(new_page_array, theClass->m_page_array, pom_class_page_buf_len(theClass->m_page_array_size));
        }

        mem_free(theClass->m_alloc, theClass->m_page_array);
        theClass->m_page_array = (void **)new_page_array;
        theClass->m_page_array_capacity = new_page_array_capacity;
    }

    return 0;
}

int pom_class_add_new_page(
    struct pom_class * theClass,
    void * page,
    error_monitor_t em)
{
    int32_t newRangeStart;
    cpe_ba_t alloc_arry;
    struct pom_data_page_head * head;

    head = (struct pom_data_page_head *)page;

    if (pom_class_reserve_page_array_slot(theClass, em) != 0) return -1;

    alloc_arry = (cpe_ba_t)(head + 1);

    head->m_magic = POM_PAGE_MAGIC;
    head->m_classId = theClass->m_id;
    head->m_reserve = 0;
    head->m_page_idx = theClass->m_page_array_size;
    head->m_obj_per_page = (uint32_t)theClass->m_object_per_page;
    cpe_ba_set_all(alloc_arry, theClass->m_object_per_page, cpe_ba_false);

    newRangeStart = (int32_t)(theClass->m_object_per_page * theClass->m_page_array_size);
    cpe_urange_put_urange(&theClass->m_urange_alloc, newRangeStart, newRangeStart + theClass->m_object_per_page);

    theClass->m_page_array[theClass->m_page_array_size] = page;
    ++theClass->m_page_array_size;

    return 0;
}

int pom_class_add_old_page(
    struct pom_class * theClass,
    void * page,
    error_monitor_t em)
{
    cpe_ba_t alloc_arry;
    struct pom_data_page_head * head;

    head = (struct pom_data_page_head *)page;

    if (pom_class_reserve_page_array_slot(theClass, em) != 0) return -1;

    alloc_arry = pom_class_ba_of_page(page);

    assert(head->m_obj_per_page == theClass->m_object_per_page);

    if (cpe_urange_put_from_ba(
            &theClass->m_urange_alloc,
            alloc_arry,
            (int32_t)(head->m_obj_per_page * theClass->m_page_array_size),
            theClass->m_object_per_page,
            cpe_ba_false) != 0)
    {
        size_t i;
        CPE_ERROR_EX(em, pom_no_memory, "alloc page urange no memory!");

        cpe_urange_mgr_clear(&theClass->m_urange_alloc);
        for(i = 0; i < theClass->m_page_array_size; ++i) {
            cpe_urange_put_from_ba(
                &theClass->m_urange_alloc,
                pom_class_ba_of_page(theClass->m_page_array[i]),
                (int32_t)(theClass->m_object_per_page * i),
                theClass->m_object_per_page,
                cpe_ba_false);
        }

        return -1;
    }

    assert(theClass->m_page_array_size + 1 < theClass->m_page_array_capacity);
    theClass->m_page_array[theClass->m_page_array_size] = page;
    ++theClass->m_page_array_size;

    return 0;
}

int32_t
pom_class_alloc_object(struct pom_class * theClass) {
    ptr_uint_t r;

    assert(theClass);

    if (cpe_urange_get_one(&theClass->m_urange_alloc, &r) == 0) {
        cpe_ba_t alloc_arry;
        int32_t pagePos;

        pagePos = (int32_t)(r / theClass->m_object_per_page);
        assert(pagePos >= 0 && (size_t)pagePos < theClass->m_page_array_size);

        alloc_arry = pom_class_ba_of_page(theClass->m_page_array[pagePos]);

        assert(cpe_ba_get(alloc_arry, r % theClass->m_object_per_page) == cpe_ba_false);
        cpe_ba_set(alloc_arry, r % theClass->m_object_per_page, cpe_ba_true);

        return (int32_t)r;
    }
    else {
        return -1;
    }
}

void pom_class_free_object(struct pom_class * theClass, int32_t oid, error_monitor_t em) {
    cpe_ba_t alloc_arry;
    int32_t pagePos;
    assert(theClass);

    assert(oid == (oid & 0xFFFFFF));

    pagePos = (int32_t)(oid / theClass->m_object_per_page);

    if(pagePos < 0 || (size_t)pagePos >= theClass->m_page_array_size) {
        CPE_ERROR_EX(
            em, pom_invalid_oid, "class %s: page pos %d overflow, page count is %zu!", 
            cpe_hs_data(theClass->m_name), pagePos, theClass->m_page_array_size);
        return;
    }

    if (cpe_urange_put_one(&theClass->m_urange_alloc, oid) != 0) {
        CPE_ERROR_EX(em, pom_no_memory, "no memory: free to urange fail!")
        return;
    }

    alloc_arry = pom_class_ba_of_page(theClass->m_page_array[pagePos]);
    cpe_ba_set(alloc_arry, oid % theClass->m_object_per_page, cpe_ba_false);
}

void * pom_class_get_object(struct pom_class * theClass, int32_t oid, error_monitor_t em) {
    cpe_ba_t alloc_arry;
    int32_t pagePos;
    int32_t posInPage;
    char * page;
    assert(theClass);

    pagePos = (int32_t)(oid / theClass->m_object_per_page);

    if(pagePos < 0 || (size_t)pagePos >= theClass->m_page_array_size) {
        CPE_ERROR_EX(
            em, pom_invalid_oid, "class %s: page pos %d overflow, page count is %zu!", 
            cpe_hs_data(theClass->m_name), pagePos, theClass->m_page_array_size);
        return NULL;
    }

    posInPage = oid % theClass->m_object_per_page;
    page = (char*)theClass->m_page_array[pagePos];

    alloc_arry = pom_class_ba_of_page(page);
    if (cpe_ba_get(alloc_arry, posInPage) != cpe_ba_true) {
        CPE_ERROR_EX(
            em, pom_invalid_oid, "class %s: oid %d not alloced!", 
            cpe_hs_data(theClass->m_name), oid);
        return NULL;
    }

    return (void*)(page + theClass->m_object_buf_begin_in_page + (theClass->m_object_size * posInPage));
}

int32_t pom_class_addr_2_object(struct pom_class *cls, void * page, void * addr) {
    int pos_to_start;
    int idx_in_page;
    struct pom_data_page_head * head;

    head = (struct pom_data_page_head *)page;
    if (head->m_page_idx < 0
        || (size_t)head->m_page_idx >= cls->m_page_array_size
        || page != cls->m_page_array[head->m_page_idx])
        return -1;

    pos_to_start = (int)((char*)addr - (((char *)page) + cls->m_object_buf_begin_in_page));

    if (pos_to_start % cls->m_object_size != 0) return -1;

    idx_in_page = pos_to_start / cls->m_object_size;

    return (int)(head->m_page_idx * cls->m_object_per_page + idx_in_page);
}

pom_class_id_t pom_class_id(pom_class_t cls) {
    return cls->m_id;
}

const char * pom_class_name(pom_class_t cls) {
    return cpe_hs_data(cls->m_name);
}

cpe_hash_string_t pom_class_name_hs(pom_class_t cls) {
    return cls->m_name;
}

size_t pom_class_obj_size(pom_class_t cls) {
    return cls->m_object_size;
}

size_t pom_class_page_size(pom_class_t cls) {
    return cls->m_page_size;
}

size_t pom_class_object_per_page(pom_class_t cls) {
    return cls->m_object_per_page;
}

struct pom_class_obj_it_data {
    pom_class_t m_class;
    uint32_t m_page_pos;
    uint32_t m_pos_in_page;
};

static void pom_class_obj_it_search_next(struct pom_class_obj_it_data * data) {
    while(data->m_page_pos < data->m_class->m_page_array_size) {
        char * page;
        cpe_ba_t alloc_arry;

        page = (char*)data->m_class->m_page_array[data->m_page_pos];
        alloc_arry = pom_class_ba_of_page(page);

        while(data->m_pos_in_page < data->m_class->m_object_per_page) {
            if (cpe_ba_get(alloc_arry, data->m_pos_in_page) == cpe_ba_true) return;
            ++data->m_pos_in_page;
        }

        data->m_pos_in_page = 0;
        ++data->m_page_pos;
    }
}

static void * pom_class_obj_it_next(pom_obj_it_t it) {
    struct pom_class_obj_it_data * data;
    char * page;
    void * r;

    data = (struct pom_class_obj_it_data *)it->m_data;

    assert(data);
    assert(data->m_class);
    assert(sizeof(struct pom_class_obj_it_data) <= sizeof(it->m_data));

    if (data->m_page_pos >= data->m_class->m_page_array_size) return NULL;
    if (data->m_pos_in_page >= data->m_class->m_object_per_page) return NULL;

    page = (char*)data->m_class->m_page_array[data->m_page_pos];

    r = (void*)(page + data->m_class->m_object_buf_begin_in_page + (data->m_class->m_object_size * data->m_pos_in_page));

    ++data->m_pos_in_page;
    pom_class_obj_it_search_next(data);

    return r;
}

void pom_class_objects(pom_class_t the_class, pom_obj_it_t it) {
    struct pom_class_obj_it_data * data;

    assert(the_class);
    assert(sizeof(struct pom_class_obj_it_data) <= sizeof(it->m_data));

    data = (struct pom_class_obj_it_data *)it->m_data;
    data->m_class = the_class;
    data->m_page_pos = 0;
    data->m_pos_in_page = 0;
    it->next = pom_class_obj_it_next;

    pom_class_obj_it_search_next(data);
}
