product:=cpe_fdb_tool
$(product).type:=progn 
$(product).depends:=argtable2 cpe_fdb cpe_dr_meta_inout
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(eval $(call product-def,$(product),tools))

