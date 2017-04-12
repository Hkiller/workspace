product:=testenv.ui_sprite_fsm
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.ui_sprite ui_sprite_fsm
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
