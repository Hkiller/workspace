dev-env-list+=mingw

MINGW_PREFIX?=i686-pc-mingw32-
mingw.GCC?=$(MINGW_PREFIX)gcc
mingw.CC?=$(MINGW_PREFIX)gcc
mingw.CXX?=$(MINGW_PREFIX)g++
mingw.AR?=$(MINGW_PREFIX)ar

mingw.CFLAGS+= -std=gnu99
mingw.CXXFLAGS+=

mingw.linker.c:=$(mingw.GCC)
mingw.linker.cpp:=$(mingw.CXX)
mingw.linker.obj-c:=$(mingw.GCC)
mingw.linker.obj-cpp=$(mingw.CXX)

ifneq ($(DEBUG),0)
mingw.CFLAGS+=-ggdb
mingw.CXXFLAGS+=-ggdb
endif

mingw.LDFLAGS.share:=--shared -Xlinker --no-undefined

mingw.default-lib-type:=static
mingw.make-static-lib-name=lib$1.a
mingw.make-dynamic-lib-name=$1.dll
mingw.make-executable-name=$1.exe
mingw.export-symbols=$(addprefix -u ,$(foreach m,$1,_$m))
mingw.env-inc=mingw
mingw.regular-path=$1
