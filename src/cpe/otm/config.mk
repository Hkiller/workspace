product:=cpe_otm
$(product).type:=lib
$(product).depends:=cpe_utils
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
