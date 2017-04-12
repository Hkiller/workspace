product:=ui_app_manip
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_sprite_cfg ui_model
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
