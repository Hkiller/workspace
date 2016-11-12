product:=gd_net
$(product).type:=lib
$(product).depends:=gd_app
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.libraries:=$($(product).c.libraries)
$(product).product.c.flags.ld:=$($(product).c.flags.ld)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
