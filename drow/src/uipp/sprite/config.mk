product:=uipp_sprite
$(product).type:=lib
$(product).depends:=ui_sprite
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
