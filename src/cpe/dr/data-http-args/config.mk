product:=cpe_dr_data_http_args
$(product).type:=lib
$(product).depends:=cpe_dr cpe_utils
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
