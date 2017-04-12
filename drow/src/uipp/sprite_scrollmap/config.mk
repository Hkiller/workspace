product:=uipp_sprite_scrollmap
$(product).type:=lib
$(product).depends:=uipp_sprite \
                    ui_sprite_scrollmap
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
