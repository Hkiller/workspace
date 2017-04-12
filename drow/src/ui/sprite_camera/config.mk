product:=ui_sprite_camera
$(product).type:=cpe-dr lib
$(product).depends:=ui_sprite render_utils ui_sprite_fsm ui_sprite_2d ui_sprite_render
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/sprite_camera/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/ui/sprite_camera
$(product).cpe-dr.data.h.with-traits:=metalib_ui_sprite_camera_traits.cpp
$(product).cpe-dr.data.c.output:=metalib_ui_sprite_camera.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_sprite_camera

$(eval $(call product-def,$(product)))
