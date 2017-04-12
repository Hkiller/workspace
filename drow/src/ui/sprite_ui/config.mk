product:=ui_sprite_ui
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_plugin_ui ui_sprite_basic ui_sprite_render
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
