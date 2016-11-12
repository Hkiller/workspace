product:=cpe_dr_meta_validate
$(product).type:=lib
$(product).depends:=cpe_dr
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
