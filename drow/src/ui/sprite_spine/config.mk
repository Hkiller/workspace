product:=ui_sprite_spine
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_sprite_2d ui_sprite_render ui_plugin_spine ui_sprite_tri
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
