product:=ui_sprite_sound
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_cache ui_runtime
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
