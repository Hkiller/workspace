product:=cpe_spack_tool
$(product).type:=progn 
$(product).depends:=argtable2 cpe_spack
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(eval $(call product-def,$(product),tools))

