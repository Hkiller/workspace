dev-env-list+=linux32

linux32.GCC?=$(if $(GCC),$(GCC),$(shell which gcc))
linux32.CC?=$(if $(GCC),$(GCC),$(shell which gcc))
linux32.CXX?=$(if $(CXX),$(CXX),$(shell which g++))
linux32.AR?=$(if $(AR),$(AR),$(shell which ar))

linux32.CFLAGS+=-m32 -fPIC
linux32.CXXFLAGS+=-m32 -fPIC

linux32.linker.c:=$(linux32.GCC)
linux32.linker.cpp:=$(linux32.CXX)
linux32.linker.obj-c:=$(linux32.GCC)
linux32.linker.obj-cpp=$(linux32.CXX)

ifneq ($(DEBUG),0)
linux32.CFLAGS+=-ggdb
linux32.CXXFLAGS+=-ggdb
endif

linux32.LDFLAGS:=-z defs
linux32.LDFLAGS.share:=--shared -z defs 

linux32.default-lib-type:=dynamic
linux32.make-static-lib-name=lib$1.a
linux32.make-dynamic-lib-name=lib$1.so
linux32.make-executable-name=$1
linux32.export-symbols=$(addprefix -u , $1)

linux32.env-inc=linux32
linux32.regular-path=$1
