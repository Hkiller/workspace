product:=appsvr_facebook
$(product).type:=lib
$(product).depends:=appsvr_account
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).android.depends:=android_support_v7
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-dir+=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml
$(product).android.android-libs:=$(product-base)/android/android-lib
$(product).android.res-dir+=$(product-base)/android/res

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
