product:=appsvr_device
$(product).type:=lib cpe-dr
$(product).depends:=plugin_app_env
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/appsvr_device/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/appsvr/device
$(product).cpe-dr.data.h.with-traits:=traits_appsvr_device.cpp
$(product).cpe-dr.data.c.output:=metalib_appsvr_device.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_appsvr_device

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-dir+=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm) $(wildcard $(product-base)/ios/*.m)

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
