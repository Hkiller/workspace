product:=cpe_dr_data_pbuf
$(product).type:=lib
$(product).depends:=cpe_dr cpe_utils
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
