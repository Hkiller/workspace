product:=uipp_sprite_cfg
$(product).type:=lib
$(product).depends:=ui_sprite_cfg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
