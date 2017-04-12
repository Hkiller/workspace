product:=testenv.ui_sprite_2d
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.ui_sprite ui_sprite_2d testenv.ui_sprite_fsm render_utils
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
