product:=plugin_app_env
$(product).type:=cpe-dr lib
$(product).depends:=gd_app
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).product.c.includes:=drow/include

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/app_env/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/plugin/app_env
$(product).cpe-dr.data.h.with-traits:=traits_plugin_app_env.cpp
$(product).cpe-dr.data.c.output:=metalib_plugin_app_env.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_plugin_app_env

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/jni/*.cpp) \
							  $(wildcard $(product-base)/android/*.cpp) \
							  $(wildcard $(product-base)/android/utils/*.cpp)
$(product).android.product.c.libraries:=android log

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.cpp)

#flex
$(product).flex.c.sources:=$(wildcard $(product-base)/flex/*.cpp)

$(eval $(call product-def,$(product)))
