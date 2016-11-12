product:=cpe_cfg_tool
$(product).type:=progn 
$(product).depends:=argtable2 cpe_cfg
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(eval $(call product-def,$(product),tools))

