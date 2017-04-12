product:=ui_sprite_render
$(product).type:=lib
$(product).depends:=plugin_app_env ui_runtime ui_sprite ui_sprite_fsm ui_sprite_2d
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
