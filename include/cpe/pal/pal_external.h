#ifndef CPE_PAL_EXTERNAL_H
#define CPE_PAL_EXTERNAL_H

#ifdef _MSC_VER

#  define IMPORT_DIRECTIVE __declspec(dllimport) 
#  define EXPORT_DIRECTIVE __declspec(dllexport)
#  define CALL __stdcall
#  define INITIALIZER(f)                                              \
      static void __cdecl f(void);                                    \
      __declspec(allocate(".CRT$XCU")) void (__cdecl*f##_)(void) = f; \
      static void __cdecl f(void)

#else /*_MSC_VER*/

#  define IMPORT_DIRECTIVE __attribute__((__visibility__("default")))
#  define EXPORT_DIRECTIVE __attribute__ ((visibility("default")))
#  define CALL  
#  define INITIALIZER(f)                                \
      static void f(void) __attribute__((constructor));   \
      static void f(void)

#endif


#endif
