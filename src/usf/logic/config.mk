product:=usf_logic
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_dr_data_basic cpe_dr_data_cfg gd_app gd_timer
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
