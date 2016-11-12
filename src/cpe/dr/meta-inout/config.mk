product:=cpe_dr_meta_inout
$(product).type:=lib
$(product).depends:=xml2 cpe_utils cpe_dr cpe_vfs cpe_dr_data_yaml
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
