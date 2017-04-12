product:=cpe_dr_data_cfg
$(product).type:=lib
$(product).depends:=cpe_dr cpe_utils cpe_cfg
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
