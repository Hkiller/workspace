product:=ui_sprite_touch
$(product).type:=cpe-dr lib
$(product).depends:=ui_sprite ui_sprite_cfg ui_sprite_2d ui_sprite_render
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/sprite_touch/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/ui/sprite_touch
$(product).cpe-dr.data.h.with-traits:=metalib_ui_sprite_touch_traits.cpp
$(product).cpe-dr.data.c.output:=metalib_ui_sprite_touch.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_sprite_touch

$(eval $(call product-def,$(product)))
