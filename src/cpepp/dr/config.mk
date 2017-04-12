product:=cpepp_dr
$(product).type:=lib
$(product).depends:=cpepp_utils cpe_dr cpe_dr_meta_inout cpe_dr_data_json cpe_dr_data_pbuf cpe_dr_data_cfg cpe_dr_data_basic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
