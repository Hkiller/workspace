product:=ui_plugin_barrage
$(product).type:=cpe-dr lib
$(product).depends:=render_utils ui_model ui_cache ui_plugin_chipmunk ui_plugin_particle cpe_dr_data_pbuf
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:=$(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/barrage/,\
                                   $(shell cat $(product-base)/protocol.def) \
                                 )
$(product).cpe-dr.data.h.output:=inc/protocol/plugin/barrage
$(product).cpe-dr.data.c.output:=metalib_plugin_barrage.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_plugin_barrage

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))

.POHEY: $(product).protocol
drow_render.export: $(product).protocol
$(product).protocol: editor.$(product)
	rsync $(call c-source-dir-to-binary-dir,$(r.ui_plugin_barrage.base),editor)/inc/protocol $(DROW_RENDER_BIN) \
			--include "*.h" -r -d
