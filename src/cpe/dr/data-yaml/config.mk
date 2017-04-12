product:=cpe_dr_data_yaml
$(product).type:=lib
$(product).depends:=cpe_dr cpe_utils yaml
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
