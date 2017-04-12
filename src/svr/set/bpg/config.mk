product:=set_bpg
$(product).type:=lib
$(product).depends:=set_share usf_bpg_pkg
$(product).c.export-symbols:=$(product-base)/symbols.def
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
