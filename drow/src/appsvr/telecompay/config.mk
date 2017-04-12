product:=appsvr_telecompay
$(product).type:=lib
$(product).depends:=appsvr_payment
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.assets-dir:=$(product-base)/android/assets
$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar) 
$(product).android.native-libs:=$(addprefix $(product-base)/android/libs/,\
                                     libegamepay_dr2.so \
                                     libypiap.so)

$(product).android.java-dir:=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/noop/*.c)

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
