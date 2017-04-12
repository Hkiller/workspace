product:=cpe_dr_data_json
$(product).type:=lib
$(product).depends:=cpe_dr cpe_utils yajl
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
