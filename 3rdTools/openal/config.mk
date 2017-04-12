product:=openal
$(product).type:=virtual

$(product).android.product.c.includes:=3rdTools/openal/android
$(product).android.mk:=$(product-base)/android/Android.mk

$(product).ios.product.c.frameworks:=AudioToolbox

$(eval $(call product-def,$(product)))
