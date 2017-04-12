product:=appsvr_damai
$(product).type:=lib
$(product).depends:=plugin_app_env appsvr_payment appsvr_account
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.assets-dir:=$(product-base)/android/DM_SdkLib/assets
$(product).android.java-dir+=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml
$(product).android.android-libs:=$(product-base)/android/DM_SdkLib
#$(product).android.native-libs:=$(addprefix $(product-base)/android/libs/,libentryexstd.so libplugin_phone.so libyt.so)
#$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar)
$(product).android.addition-datas:=YT_APPID

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
