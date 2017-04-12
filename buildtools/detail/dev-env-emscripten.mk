dev-env-list+=emscripten

emscripten.CC?=$(shell which emcc)
emscripten.CXX?=$(shell which em++)
emscripten.AR?=$(shell which emar)

emscripten.CPPFLAGS+=-DEMSCRIPTEN
emscripten.CFLAGS+=
emscripten.CXXFLAGS+=

ifneq ($(DEBUG),0)
emscripten.CFLAGS+=-g
emscripten.CXXFLAGS+=-g
endif

emscripten.LDFLAGS:=
emscripten.LDFLAGS.share:=

emscripten.env-inc=emscripten
emscripten.regular-path=$(if $(filter cygwin,$(OS_NAME)),$(shell cygpath -m $1),$1)

emscripten.default-lib-type:=static
emscripten.make-static-lib-name=lib$1.a
emscripten.make-dynamic-lib-name=$1.js
emscripten.make-executable-name=$1.js
emscripten.export-symbols=$(addprefix -u ,$(foreach m,$1,_$m))
