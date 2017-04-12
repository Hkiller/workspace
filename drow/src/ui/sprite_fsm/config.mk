product:=ui_sprite_fsm
$(product).type:=cpe-dr lib
$(product).depends:=cpe_xcalc ui_sprite
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/sprite_fsm/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/ui/sprite_fsm
$(product).cpe-dr.data.h.with-traits:=metalib_ui_sprite_fsm_traits.cpp
$(product).cpe-dr.data.c.output:=metalib_ui_sprite_fsm.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_sprite_fsm

$(eval $(call product-def,$(product)))
