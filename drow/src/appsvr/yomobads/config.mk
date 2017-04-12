product:=appsvr_yomobads
$(product).type:=lib
$(product).depends:=appsvr_ad
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android
$(product).android.depends:=android_support_v4
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-dir:=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml
$(product).android.assets-dir:=$(product-base)/android/assets
$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar)
$(product).android.res-dir+=$(product-base)/android/res

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm)

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
