product:=cpe_utils
$(product).type:=lib
$(product).depends:=cpe_pal
$(product).product.c.includes:=include
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).linux32.product.c.libraries:=m
$(product).linux64.product.c.libraries:=m

$(eval $(call product-def,$(product)))
