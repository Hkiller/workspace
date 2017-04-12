product:=android_support_v4
$(product).type:=virtual
$(product).android.java-libs:=$(wildcard $(product-base)/*.jar)
$(eval $(call product-def,$(product)))
