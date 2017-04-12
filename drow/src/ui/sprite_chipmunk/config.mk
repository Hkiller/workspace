product:=ui_sprite_chipmunk
$(product).type:=cpe-dr lib
$(product).depends:=pcre2 ui_sprite ui_sprite_fsm ui_sprite_2d ui_sprite_cfg ui_plugin_chipmunk ui_sprite_render ui_sprite_tri
$(product).product.c.output-includes:=inc
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/sprite_chipmunk/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/ui/sprite_chipmunk
$(product).cpe-dr.data.c.output:=metalib_ui_sprite_chipmunk_model.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_sprite_chipmunk

$(eval $(call product-def,$(product),editor))

.POHEY: $(product).protocol
drow_render.export: $(product).protocol
$(product).protocol: editor.$(product)
	rsync $(call c-source-dir-to-binary-dir,$(r.ui_sprite_chipmunk.base),editor)/inc/protocol $(DROW_RENDER_BIN) \
			--include "*.h" -r -d
