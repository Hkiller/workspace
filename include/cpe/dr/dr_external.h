#ifndef CPE_DR_EXTERNAL_H
#define CPE_DR_EXTERNAL_H
#include "cpe/pal/pal_external.h"

#if defined CPE_DR_DLL
#    if defined CPE_DR_BUILD_DLL
#        define CPE_DR_API EXPORT_DIRECTIVE
#    else
#        define CPE_DR_API IMPORT_DIRECTIVE
#    endif
#else
#    define CPE_DR_API
#endif
  
#endif
