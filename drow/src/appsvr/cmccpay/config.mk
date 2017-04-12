product:=appsvr_cmccpay
$(product).type:=lib
$(product).depends:=appsvr_payment
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.assets-dir:=$(product-base)/android/assets
$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar) 
$(product).android.java-runtime-libs:=$(wildcard $(product-base)/android/runtime/*.jar) 
$(product).android.native-libs:=$(product-base)/android/libs/libmegjb.so $(product-base)/android/libs/libmg20pbase.so
$(product).android.java-dir+=$(product-base)/android/src
$(product).android.res-dir+=$(product-base)/android/res
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/noop/*.c)

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
