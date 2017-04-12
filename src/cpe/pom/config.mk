product:=cpe_pom
$(product).type:=lib
$(product).depends:=cpe_utils
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
