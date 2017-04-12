product:=appsvr_qihoo
$(product).type:=lib
$(product).depends:=appsvr_payment appsvr_account
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android 1.3.0(468)
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.assets-dir:=$(product-base)/android/assets
$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar)
$(product).android.java-dir+=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml
$(product).android.addition-datas:=QHOPENSDK_APPID QHOPENSDK_APPKEY QHOPENSDK_APPKEY QHOPENSDK_PRIVATEKEY QHOPENSDK_WEIXIN_APPID

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
