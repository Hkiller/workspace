dev-env-list+=android

android.GCC?=$(if $(GCC),$(GCC),$(shell which gcc))
android.CC?=$(if $(GCC),$(GCC),$(shell which gcc))
android.CXX?=$(if $(CXX),$(CXX),$(shell which g++))
android.AR?=$(if $(AR),$(AR),$(shell which ar))

#android.CFLAGS+=-m32 -fPIC
#android.CXXFLAGS+=-m32 -fPIC

android.linker.c:=$(android.GCC)
android.linker.cpp:=$(android.CXX)
android.linker.obj-c:=$(android.GCC)
android.linker.obj-cpp=$(android.CXX)

android.LDFLAGS:=-z defs
android.LDFLAGS.share:=--shared -z defs 

android.env-inc=android
android.regular-path=$1

android.default-lib-type:=dynamic
android.make-static-lib-name=lib$1.a
android.make-dynamic-lib-name=lib$1.so
android.make-executable-name=$1
android.export-symbols=$(addprefix -u , $1)

android.lib.iconv:=

android.lib.dl?=dl
