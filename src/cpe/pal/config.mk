product:=cpe_pal
cpe_pal.type:=lib
$(product).product.c.includes:=include
cpe_pal.c.libraries:=
cpe_pal.c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
