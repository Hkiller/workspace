product:=appsvr_umeng
$(product).type:=lib
$(product).depends:=plugin_app_env ui_plugin_ui
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar)
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml
#$(product).android.addition-datas:=UMENG_APPKEY UMENG_CHANNEL
$(product).android.java-dir+=$(product-base)/android/src

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm)
$(product).ios.c.libraries:=MobClickGameLibrary

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
