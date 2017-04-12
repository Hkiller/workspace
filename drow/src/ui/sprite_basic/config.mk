product:=ui_sprite_basic
$(product).type:=cpe-dr lib
$(product).depends:=cpe_xcalc ui_sprite ui_sprite_fsm ui_sprite_cfg
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/sprite_basic/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/ui/sprite_basic
$(product).cpe-dr.data.h.with-traits:=metalib_ui_sprite_basic_traits.cpp
$(product).cpe-dr.data.c.output:=metalib_ui_sprite_basic.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_sprite_basic

$(eval $(call product-def,$(product)))
