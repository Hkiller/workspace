#ifndef CPE_POM_ERROR_H
#define CPE_POM_ERROR_H
#include "pom_types.h"

enum pom_error {
    pom_success = 0
    , pom_no_memory
    , pom_no_buffer
    , pom_buffer_get_fail
    , pom_invalid_oid
    , pom_invalid_align
    , pom_invalid_address
    , pom_page_head_error
    , pom_page_size_too_big
    , pom_page_size_too_small
    , pom_class_not_exist
    , pom_class_name_too_long
    , pom_class_overflow
    , pom_class_name_duplicate
};

#endif
