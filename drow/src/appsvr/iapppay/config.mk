product:=appsvr_iapppay
$(product).type:=lib
$(product).depends:=appsvr_payment
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar)
$(product).android.native-libs:=$(product-base)/android/libs/libentryexstd.so
$(product).android.java-dir:=$(product-base)/android/src
$(product).android.res-dir:=$(product-base)/android/res
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm)
$(product).ios.product.c.frameworks:=IapppayKit
$(product).ios.product.c.framework-pathes:=$(product-base)/ios/libs

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
