product:=cpe_dp
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_nm cpe_dr_data_json
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
