#include "cpe/utils/algorithm.h"

void * cpe_lower_bound(void * first, size_t nel, const void * key, size_t width, int (*compar)(const void *, const void *)) {
      size_t half;
      void * middle;

      while (nel > 0) {
          int cmp_result;

          half = nel >> 1;
          middle = ((char*)(first)) + (half * width);

          cmp_result = compar(middle, key);

          if (cmp_result < 0) {
              first = ((char*)middle) + width;
              nel = nel - half - 1;
          }
          else {
              nel = half;
          }
      }

      return first;
}

void * cpe_upper_bound(void * first, size_t nel, const void * key, size_t width, int (*compar)(const void *, const void *)) {
    size_t half;
    void * middle;

    while (nel > 0) {
        int cmp_result;

        half = nel >> 1;
        middle = ((char*)first) + (half * width);

        cmp_result = compar(middle, key);
        if (cmp_result > 0) {
            nel = half;
        }
        else {
            first = ((char*)middle) + width;
            nel = nel - half - 1;
	    }
	}

    return first;
}

int cpe_comap_uint32(const void * l, const void * r) {
    return (int)(*((const uint32_t *)l)) - ((int)(*(const uint32_t *)r));
}
