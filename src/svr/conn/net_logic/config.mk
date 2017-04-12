product:=conn_net_logic
$(product).type:=lib
$(product).depends:=conn_net_cli usf_logic
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
