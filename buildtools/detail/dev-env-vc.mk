dev-env-list+=vc

vc.check=$(call assert-not-null,VC_HOME) \
         $(call assert-file-exists,$(VC_HOME))

vc.GCC?=$(shell which gcc)
vc.CC=$(vc.GCC)

vc.CFLAGS+=
vc.CXXFLAGS+=

ifneq ($(DEBUG),0)
vc.CFLAGS+=-ggdb
vc.CXXFLAGS+=-ggdb
endif

vc.env-inc=vc
vc.regular-path=$1

vc.LDFLAGS.share:=--shared

vc.default-lib-type:=dynamic
vc.make-static-lib-name=$1.lib
vc.make-dynamic-lib-name=$1.dll
vc.make-executable-name=$1.exe
