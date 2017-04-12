product:=usf_bpg_net
$(product).type:=lib
$(product).depends:=cpe_net gd_vnet usf_logic usf_bpg_pkg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))



