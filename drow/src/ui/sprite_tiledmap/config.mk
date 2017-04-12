product:=ui_sprite_tiledmap
$(product).type:=lib
$(product).depends:=ui_plugin_tiledmap ui_sprite ui_sprite_fsm ui_sprite_cfg  ui_sprite_2d ui_sprite_render
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product),editor))
