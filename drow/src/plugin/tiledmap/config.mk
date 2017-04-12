product:=ui_plugin_tiledmap
$(product).type:=cpe-dr lib
$(product).depends:=ui_model render_utils ui_plugin_basicanim
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:=$(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/tiledmap/,\
                                   $(shell cat $(product-base)/protocol.def) \
                                 )
$(product).cpe-dr.data.h.output:=inc/protocol/plugin/tiledmap
$(product).cpe-dr.data.c.output:=metalib_plugin_tiledmap.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_plugin_tiledmap

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))

.POHEY: $(product).protocol
drow_render.export: $(product).protocol
$(product).protocol: editor.$(product)
	rsync $(call c-source-dir-to-binary-dir,$(r.ui_plugin_tiledmap.base),editor)/inc/protocol $(DROW_RENDER_BIN) \
			--include "*.h" -r -d
