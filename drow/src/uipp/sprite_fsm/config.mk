product:=uipp_sprite_fsm
$(product).type:=lib
$(product).depends:=uipp_sprite ui_sprite_fsm
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
