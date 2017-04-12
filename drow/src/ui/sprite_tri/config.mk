product:=ui_sprite_tri
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_sprite_cfg
$(product).product.c.output-includes:=inc
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product),editor))
