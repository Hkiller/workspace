product:=cpe_dr_tool_lib
$(product).type:=lib 
$(product).depends:=argtable2 cpe_dr_meta_inout cpe_dr_meta_validate cpe_dr 
$(product).c.libraries:=
$(product).c.sources:= $(filter-out %/main.c,$(wildcard $(product-base)/*.c))
$(product).c.lib.type:=static 
$(eval $(call product-def,$(product)))

product:=cpe_dr_tool
$(product).type:=progn
$(product).depends:=cpe_dr_tool_lib
$(product).c.sources:=$(product-base)/main.c
$(eval $(call product-def,$(product),tools))

