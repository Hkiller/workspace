product:=appsvr_chuangku
$(product).type:=lib cpe-dr
$(product).depends:=gd_timer gd_dr_store gd_app_attr appsvr_payment appsvr_sdk
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/appsvr_chuangku/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/appsvr/chuangku
$(product).cpe-dr.data.h.with-traits:=traits_appsvr_chuangku.cpp
$(product).cpe-dr.data.c.output:=metalib_appsvr_chuangku.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_appsvr_chuangku

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.assets-dir:=$(product-base)/android/assets
$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar) 
$(product).android.java-dir:=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/noop/*.c)

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
