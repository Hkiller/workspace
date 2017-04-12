product:=gd_dr_cvt
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_dr cpe_dr_data_pbuf cpe_dr_data_bson gd_app
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
