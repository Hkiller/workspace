product:=cpe_spack
$(product).type:=lib
$(product).depends:=cpe_utils cpe_vfs
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
