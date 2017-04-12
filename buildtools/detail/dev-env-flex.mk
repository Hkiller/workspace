dev-env-list+=flex

#AS3COMPILER:=asc2.jar
flex.check=$(call assert-not-null,FLASCC)

JVMARGS?=-Xmx16G

flex.0.as3-flags:=
flex.0.LDFLAGS:=-O0 -g
flex.0.CPPFLAGS:=-O0 -g
flex.1.as3-flags:=-config CONFIG::DEBUG=true
flex.1.LDFLAGS:=-O3
flex.1.CPPFLAGS:=-O3
flex.2.as3-flags:=-config CONFIG::DEBUG=false
flex.2.LDFLAGS:=-O4 -disable-telemetry
flex.2.CPPFLAGS:=-O4


flex.as3-comiler=java $(JVMARGS) -jar $(call flex.regular-path,$(FLASCC)/usr/lib/asc2.jar) $(flex.$(flex).as3-flags) -merge -md
flex.swig=java $(JVMARGS) -jar $(call flex.regular-path,$(FLASCC)/usr/lib/as3wig.jar)
flex.vfs-generator=$(FLASCC)/usr/bin/genfs

flex.GCC?=$(FLASCC)/usr/bin/gcc
flex.CC?=$(FLASCC)/usr/bin/gcc
flex.CXX?=$(FLASCC)/usr/bin/g++
flex.AR?=$(FLASCC)/usr/bin/ar

flex.CPPFLAGS+=-DFLEX $(flex.$(flex).CPPFLAGS)
flex.CFLAGS+=-Wno-write-strings -Wno-trigraphs -pthread
flex.CXXFLAGS+=

flex.linker.c:=$(flex.GCC) -jvmopt="$(JVMARGS)" -jvmopt="-XX:-UseGCOverheadLimit"
flex.linker.cpp:=$(flex.CXX) -jvmopt="$(JVMARGS)" -jvmopt="-XX:-UseGCOverheadLimit"

ifneq ($(DEBUG),0)
flex.CFLAGS+=
flex.CXXFLAGS+=
endif

flex.LDFLAGS:=-lAS3++ -lFlash++ $(flex.$(flex).LDFLAGS) -no-swf-preloader -emit-swf -swf-version=18 -pthread
flex.LDFLAGS.share:=

flex.env-inc=flex
flex.regular-path=$(if $(filter cygwin,$(OS_NAME)),$(shell cygpath -m $1),$1)

flex.default-lib-type:=static
flex.make-static-lib-name=lib$1.a
flex.make-dynamic-lib-name=$1.swc
flex.make-executable-name=$1.swf
#flex.export-symbols=$(addprefix -u , $1)

compiler.gcc-flex.flags.warning=
compiler.gcc-flex.flags.gen-dep=-MMD -MF
