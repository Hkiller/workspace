product:=usf_bpg_rsp
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils gd_dr_cvt gd_app usf_logic gd_dr_store usf_bpg_pkg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=carry
$(product).cpe-dr.carry.generate:=h c
$(product).cpe-dr.carry.source:=$(product-base)/bpg_rsp_carry_info.xml \
                                $(product-base)/bpg_rsp_addition.xml
$(product).cpe-dr.carry.h.output:=protocol/bpg_rsp
$(product).cpe-dr.carry.c.output:=protocol/bpg_rsp/carry_package.c
$(product).cpe-dr.carry.c.arg-name:=g_metalib_carry_package

$(eval $(call product-def,$(product)))



