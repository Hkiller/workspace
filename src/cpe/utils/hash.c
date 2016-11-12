#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/hash.h"

typedef struct cpe_hash_entry * cpe_hash_entry_t;
struct cpe_hash_bucket {
    cpe_hash_entry_t m_entry;
};

#define CPE_HASH_BUCKET_FREE_LIST_COUNT_PRE_ALLOC (64)

#define cpe_hash_entry_2_obj(hstable, entry)    \
    (((char*)entry) - hstable->m_obj2Entry)

#define cpe_hash_obj_2_entry(hstable, obj)    \
    ((cpe_hash_entry_t)( ((char*)obj) + hstable->m_obj2Entry ))

uint32_t cpe_hash_str(const void * str, size_t len) {
    size_t i;
	uint32_t h = (uint32_t)len;
	size_t step = (len >> 5) + 1;
	for (i = len; i >= step; i -= step) {
	    h = h ^ ((h<<5)+(h>>2)+(uint32_t)((const char *)str)[i-1]);
    }

	return h;
}

static int32_t cpe_prime_array[] = {
    17,             /* 0 */
    37,             /* 1 */
    79,             /* 2 */
    163,            /* 3 */
    331,            /* 4 */
    673,            /* 5 */
    1361,           /* 6 */
    2729,           /* 7 */
    5471,           /* 8 */
    10949,          /* 9 */
    21911,          /* 10 */
    43853,          /* 11 */
    87719,          /* 12 */
    175447,         /* 13 */
    350899,         /* 14 */
    701819,         /* 15 */
    1403641,        /* 16 */
    2807303,        /* 17 */
    5614657,        /* 18 */
    11229331,       /* 19 */
    22458671,       /* 20 */
    44917381,       /* 21 */
    89834777,       /* 22 */
    179669557,      /* 23 */
    359339171,      /* 24 */
    718678369,      /* 25 */
    1437356741,     /* 26 */
    2147483647      /* 27 (largest signed int prime) */
};               

static int32_t select_next_prime(int32_t size) {
    int i;
    for(i = 0; i < sizeof(cpe_prime_array) / sizeof(int32_t); ++i) {
        if (cpe_prime_array[i] >= size) {
            return cpe_prime_array[i];
        }
    }

    return -1;
}

int cpe_hash_table_init(
    cpe_hash_table_t hashtable,
    mem_allocrator_t alloc,
    cpe_hash_fun_t hash_fun,
    cpe_hash_eq_t eq_fun,
    int32_t obj2Entry,
    int32_t suggest_capacity)
{
    int bucketsBufSize;

    if (hashtable == NULL) return -1;

    hashtable->m_alloc = alloc;
    hashtable->m_hash_fun = hash_fun;
    hashtable->m_eq_fun = eq_fun;
    hashtable->m_user_data = NULL;
    hashtable->m_destory_fun = NULL;
    hashtable->m_destory_context = NULL;
    hashtable->m_count = 0;
    hashtable->m_obj2Entry = obj2Entry;

    /*create bulck list*/
    hashtable->m_bucket_capacity = select_next_prime(suggest_capacity);

    bucketsBufSize = sizeof(struct cpe_hash_bucket) * hashtable->m_bucket_capacity;
    hashtable->m_buckets = (struct cpe_hash_bucket *)mem_alloc(alloc, bucketsBufSize);
    if (hashtable->m_buckets == NULL) return -1;
    bzero(hashtable->m_buckets, bucketsBufSize);

    return 0;
}

void * cpe_hash_table_user_data(cpe_hash_table_t hstable) {
    return hstable->m_user_data;
}

void cpe_hash_table_set_user_data(cpe_hash_table_t hstable, void * ctx) {
    hstable->m_user_data = ctx;
}

int cpe_hash_table_count(cpe_hash_table_t hstable) {
    return hstable->m_count;
}

void cpe_hash_table_set_destory_fun(
    cpe_hash_table_t hstable,
    cpe_hash_destory_t destory_fun,
    void * destory_context)
{
    if (hstable == NULL) return;

    hstable->m_destory_fun = destory_fun;
    hstable->m_destory_context = destory_context;
}

#define cpe_hash_check_need_resize(hstable, count) (((size_t)(count)) > (hstable)->m_bucket_capacity)

int cpe_hash_table_check_resize(cpe_hash_table_t hstable, int32_t suggest_capacity) {
    if (cpe_hash_check_need_resize(hstable, suggest_capacity)) {
        return cpe_hash_table_resize(hstable, suggest_capacity);
    }
    else {
        return 0;
    }
}

int cpe_hash_table_resize(cpe_hash_table_t hstable, int32_t suggest_capacity) {
    int32_t newCapacity;
    int32_t newBucketsBufSize;
    struct cpe_hash_bucket * newBuckets;
    size_t i;

    newCapacity = select_next_prime(suggest_capacity);
    if (newCapacity < 0) return -1;

    newBucketsBufSize = sizeof(struct cpe_hash_bucket) * newCapacity;
    newBuckets = (struct cpe_hash_bucket *)mem_alloc(hstable->m_alloc, newBucketsBufSize);
    if (newBuckets == NULL) return -1;
    bzero(newBuckets, newBucketsBufSize);

    for(i = 0; i < hstable->m_bucket_capacity; ++i) {
        cpe_hash_entry_t* old_pos = &(hstable->m_buckets + i)->m_entry;
        
        while(*old_pos) {
            cpe_hash_entry_t* check_pos;
            cpe_hash_entry_t* insert_pos;
            cpe_hash_entry_t move_e;
            cpe_hash_entry_t e;

            move_e = *old_pos;
            
            check_pos = old_pos;
            insert_pos = &(newBuckets + (move_e->m_value % newCapacity))->m_entry;

            e = *check_pos;
            if (e->m_next) e->m_next->m_prev = check_pos;
            *check_pos = e->m_next;
            
            e->m_next = *insert_pos;
            if (*insert_pos) (*insert_pos)->m_prev = &e->m_next;
            
            e->m_prev = insert_pos;
            *insert_pos = e;
            
            insert_pos = &e->m_next;
            
            while(*check_pos) {
                e = *check_pos;
                
                if (hstable->m_eq_fun(cpe_hash_entry_2_obj(hstable, move_e), cpe_hash_entry_2_obj(hstable, e), hstable->m_user_data)) {
                    if (e->m_next) e->m_next->m_prev = check_pos;
                    *check_pos = e->m_next;
            
                    e->m_next = *insert_pos;
                    if (*insert_pos) (*insert_pos)->m_prev = &e->m_next;
                    e->m_prev = insert_pos;
                    *insert_pos = e;
                    
                    insert_pos = &e->m_next;
                }
                else {
                    check_pos = &e->m_next;
                }
            }
        }
    }

    mem_free(hstable->m_alloc, hstable->m_buckets);
    hstable->m_buckets = newBuckets;
    hstable->m_bucket_capacity = newCapacity;

    return 0;
}

void cpe_hash_table_fini(cpe_hash_table_t hstable) {
    if (hstable == NULL) return;

    if (hstable->m_destory_fun) {
        size_t i;
        for(i = 0; i < hstable->m_bucket_capacity; ++i) {
            cpe_hash_entry_t e = (hstable->m_buckets + i)->m_entry;
            while(e) {
                cpe_hash_entry_t n = e->m_next;
                hstable->m_destory_fun(
                    cpe_hash_entry_2_obj(hstable, e),
                    hstable->m_destory_context);
                e = n;
            }
        }
    }

    if (hstable->m_buckets) {
        mem_free(hstable->m_alloc, hstable->m_buckets);
        hstable->m_buckets = NULL;
    }
}

int cpe_hash_table_insert_noresize(cpe_hash_table_t hstable, void * obj) {
    int32_t bucketPos;
    cpe_hash_entry_t entry;
    cpe_hash_entry_t* bucket_insert;

    if (obj == NULL) return -1;

    entry = cpe_hash_obj_2_entry(hstable, obj);

    if (entry->m_prev != NULL) return -1;

    entry->m_value = hstable->m_hash_fun(obj, hstable->m_user_data);
    bucketPos = entry->m_value % hstable->m_bucket_capacity;

    bucket_insert = &(hstable->m_buckets + bucketPos)->m_entry;

    entry->m_prev = bucket_insert;
    entry->m_next = *bucket_insert;
    if (entry->m_next) {
        entry->m_next->m_prev = &entry->m_next;
    }
    *bucket_insert = entry;

    ++hstable->m_count;

    return 0;
}

int cpe_hash_table_insert_unique_noresize(cpe_hash_table_t hstable, void * obj) {
    int32_t bucketPos;
    cpe_hash_entry_t entry;
    cpe_hash_entry_t* bucket_insert;

    if (obj == NULL) return -1;

    entry = cpe_hash_obj_2_entry(hstable, obj);
    if (entry->m_prev != NULL) return -1;

    entry->m_value = hstable->m_hash_fun(obj, hstable->m_user_data);
    bucketPos = entry->m_value % hstable->m_bucket_capacity;

    bucket_insert = &(hstable->m_buckets + bucketPos)->m_entry;

    while(*bucket_insert) {
        if (hstable->m_eq_fun(
                cpe_hash_entry_2_obj(hstable, (*bucket_insert)),
                obj,
                hstable->m_user_data))
        {
            return -1;
        }

        bucket_insert = &(*bucket_insert)->m_next;
    }

    entry->m_prev = bucket_insert;
    entry->m_next = NULL;
    *bucket_insert = entry;

    ++hstable->m_count;

    return 0;
}

int cpe_hash_table_insert_unique(cpe_hash_table_t hstable, void * obj) {
    assert(hstable);

    if (cpe_hash_check_need_resize(hstable, hstable->m_count + 1)) {
        if (cpe_hash_table_resize(hstable, hstable->m_count + 1) != 0) {
            return -1;
        }
    }

    return cpe_hash_table_insert_unique_noresize(hstable, obj);
}

int cpe_hash_table_insert(cpe_hash_table_t hstable, void * obj) {
    assert(hstable);

    if (cpe_hash_check_need_resize(hstable, hstable->m_count + 1)) {
        if (cpe_hash_table_resize(hstable, hstable->m_count + 1) != 0) {
            return -1;
        }
    }

    return cpe_hash_table_insert_noresize(hstable, obj);
}

int cpe_hash_table_remove_by_key(cpe_hash_table_t hstable, const void * obj) {
    void * existObj = cpe_hash_table_find(hstable, obj);
    if (existObj) {
        int r = cpe_hash_table_remove_by_ins(hstable, existObj);
        return r == 0 ? 1 : r;
    }
    else {
        return 0;
    }
}

int cpe_hash_table_remove_all_by_key(cpe_hash_table_t hstable, const void * obj) {
    void * existObj = cpe_hash_table_find(hstable, obj);
    if (existObj) {
        int r = 0;
        cpe_hash_entry_t e;

        e = cpe_hash_obj_2_entry(hstable, existObj)->m_next;

        if (cpe_hash_table_remove_by_ins(hstable, existObj) == 0) {
            ++r;
        }

        while(e) {
            cpe_hash_entry_t n = e->m_next;
            void * checkObj = cpe_hash_entry_2_obj(hstable, e);

            if (hstable->m_eq_fun(checkObj, obj, hstable->m_user_data)) {
                if (cpe_hash_table_remove_by_ins(hstable, checkObj) == 0) {
                    ++r;
                }
            }

            e = n;
        }

        return r;
    }
    else {
        return 0;
    }
}

int cpe_hash_table_remove_by_ins(cpe_hash_table_t hstable, void * obj) {
    cpe_hash_entry_t entry;

    assert(obj);

    entry = cpe_hash_obj_2_entry(hstable, obj);
    assert(entry->m_prev);

    if (entry->m_next) entry->m_next->m_prev = entry->m_prev;
    (*entry->m_prev) = entry->m_next;

    entry->m_next = NULL;
    entry->m_prev = NULL;
    
    assert(hstable->m_count > 0);
    --hstable->m_count;

    /*destory object*/
    if (hstable->m_destory_fun) {
        hstable->m_destory_fun(obj, hstable->m_destory_context);
    }

    return 0;
}

void * cpe_hash_table_find(cpe_hash_table_t hstable, const void * obj) {
    uint32_t bucketPos;
    uint32_t hsvalue;
    cpe_hash_entry_t* find_pos;

    hsvalue = hstable->m_hash_fun(obj, hstable->m_user_data); 

    bucketPos = hsvalue % hstable->m_bucket_capacity;

    find_pos = &((hstable->m_buckets + bucketPos)->m_entry);

    while(*find_pos) {
        void * checkObj = cpe_hash_entry_2_obj(hstable, *find_pos);
        if (hstable->m_eq_fun(checkObj, obj, hstable->m_user_data)) {
            return checkObj;
        }
        find_pos = &(*find_pos)->m_next;
    }

    return NULL;
}

void * cpe_hash_table_find_next(cpe_hash_table_t hstable, const void * obj) {
    cpe_hash_entry_t entry;

    if (hstable == NULL || obj == NULL) return NULL;

    entry = cpe_hash_obj_2_entry(hstable, obj);

    for(entry = entry->m_next; entry; entry = entry->m_next) {
        void * checkObj = cpe_hash_entry_2_obj(hstable, entry);
        if (hstable->m_eq_fun(checkObj, obj, hstable->m_user_data)) {
            return checkObj;
        }
    }

    return NULL;
}

static
cpe_hash_entry_t
cpe_hash_search_next_bucket(cpe_hash_table_t hstable, int32_t startPos) {
    assert(startPos >= 0);
    for(; ((size_t)startPos) < hstable->m_bucket_capacity; ++startPos) {
        cpe_hash_entry_t entry = (hstable->m_buckets + startPos)->m_entry;
        if (entry) return entry;
    }

    return NULL;
}

void cpe_hash_it_init(cpe_hash_it_t * it, cpe_hash_table_t hstable) {
    it->m_hstable = hstable;
    it->m_entry = cpe_hash_search_next_bucket(hstable, 0);
}

void * cpe_hash_it_next(cpe_hash_it_t * it) {
    void * r;

    if (it->m_entry == NULL) return NULL;
    r = cpe_hash_entry_2_obj(it->m_hstable, it->m_entry);

    it->m_entry = 
        it->m_entry->m_next
        ? it->m_entry->m_next
        : cpe_hash_search_next_bucket(
            it->m_hstable, 
            (it->m_entry->m_value % it->m_hstable->m_bucket_capacity) + 1);

    return r;
}
