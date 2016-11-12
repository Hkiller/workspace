product:=cpe_fdb
$(product).type:=lib
$(product).depends:=cpe_utils cpe_vfs cpe_cfg cpe_dr
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
