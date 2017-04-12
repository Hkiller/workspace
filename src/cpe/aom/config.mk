product:=cpe_aom
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_dr cpe_dr_data_cfg cpe_dr_meta_inout
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
