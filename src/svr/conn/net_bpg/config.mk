product:=conn_net_bpg
$(product).type:=lib
$(product).depends:=conn_net_cli usf_bpg_pkg
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
