product:=appsvr_facebook_share
$(product).type:=lib
$(product).depends:=appsvr_share appsvr_facebook
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-dir+=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
