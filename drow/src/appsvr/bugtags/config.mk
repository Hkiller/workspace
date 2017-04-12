product:=appsvr_bugtags
$(product).type:=lib
$(product).depends:=plugin_app_env
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android
$(product).android.depends:=android_support_v4
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-dir+=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml
$(product).android.android-libs:=$(product-base)/android/android-lib

#ios (version=1.2.6)
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm)
$(product).ios.product.c.frameworks:=Bugtags
$(product).ios.product.c.framework-pathes:=$(product-base)/ios/libs

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
