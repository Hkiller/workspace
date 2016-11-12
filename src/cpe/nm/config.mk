product:=cpe_nm
$(product).type:=lib
$(product).depends:=cpe_utils
$(product).c.libraries:=
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
