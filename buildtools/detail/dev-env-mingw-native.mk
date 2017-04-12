dev-env-list+=mingw-native

MINGW_PREFIX?=i686-w64-mingw32-
mingw-native.GCC?=$(if $(dir $(MINGW_PREFIX)),cd $(dir $(MINGW_PREFIX)) && $(notdir $(MINGW_PREFIX)gcc),$(MINGW_PREFIX)gcc)
mingw-native.CC?=$(if $(dir $(MINGW_PREFIX)),cd $(dir $(MINGW_PREFIX)) && $(notdir $(MINGW_PREFIX)gcc),$(MINGW_PREFIX)gcc)
mingw-native.CXX?=$(if $(dir $(MINGW_PREFIX)),cd $(dir $(MINGW_PREFIX)) && $(notdir $(MINGW_PREFIX)g++),$(MINGW_PREFIX)g++)
mingw-native.AR?=$(if $(dir $(MINGW_PREFIX)),cd $(dir $(MINGW_PREFIX)) && $(notdir $(MINGW_PREFIX)ar),$(MINGW_PREFIX)ar)

mingw-native.CFLAGS+= -std=gnu99
mingw-native.CXXFLAGS+=

mingw-native.linker.c:=$(mingw-native.GCC)
mingw-native.linker.cpp:=$(mingw-native.CXX)
mingw-native.linker.obj-c:=$(mingw-native.GCC)
mingw-native.linker.obj-cpp=$(mingw-native.CXX)

ifneq ($(DEBUG),0)
mingw-native.CFLAGS+=-ggdb
mingw-native.CXXFLAGS+=-ggdb
endif

mingw-native.LDFLAGS.share:=--shared -Xlinker --no-undefined

mingw-native.default-lib-type:=static
mingw-native.make-static-lib-name=lib$1.a
mingw-native.make-dynamic-lib-name=$1.dll
mingw-native.make-executable-name=$1.exe
mingw-native.export-symbols=$(addprefix -u ,$(foreach m,$1,_$m))
mingw-native.env-inc=mingw
mingw-native.regular-path=$(shell cygpath -w $1)
