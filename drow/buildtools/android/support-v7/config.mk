product:=android_support_v7
$(product).type:=virtual
$(product).android.android-libs:=$(addprefix $(product-base)/,appcompat cardview gridlayout mediarouter palette preference recyclerview)
$(eval $(call product-def,$(product)))
