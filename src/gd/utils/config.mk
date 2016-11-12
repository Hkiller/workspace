product:=gd_utils
$(product).type:=lib
$(product).depends:=cpe_utils gd_app
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))



