dev-env-list+=linux64

linux64.GCC?=$(if $(GCC),$(GCC),$(shell which gcc))
linux64.CC?=$(if $(GCC),$(GCC),$(shell which gcc))
linux64.CXX?=$(if $(CXX),$(CXX),$(shell which g++))
linux64.AR?=$(if $(AR),$(AR),$(shell which ar))

linux64.CFLAGS+=-m64 -fPIC
linux64.CXXFLAGS+=-m64 -fPIC

linux64.linker.c:=$(linux64.GCC)
linux64.linker.cpp:=$(linux64.CXX)
linux64.linker.obj-c:=$(linux64.GCC)
linux64.linker.obj-cpp=$(linux64.CXX)

ifneq ($(DEBUG),0)
linux64.CFLAGS+=-ggdb
linux64.CXXFLAGS+=-ggdb
endif

linux64.env-inc=linux64
linux64.regular-path=$1

linux64.LDFLAGS:=-z defs
linux64.LDFLAGS.share:=--shared -z defs

linux64.default-lib-type:=dynamic
linux64.make-static-lib-name=lib$1.a
linux64.make-dynamic-lib-name=lib$1.so
linux64.make-executable-name=$1
linux64.export-symbols=$(addprefix -u , $1)
