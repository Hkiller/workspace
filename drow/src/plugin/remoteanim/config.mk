product:=ui_plugin_remoteanim
$(product).type:=lib
$(product).depends:=ui_runtime ui_plugin_basicanim
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:=$(product-base)/symbols.def

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))

.POHEY: $(product).protocol
drow_render.export: $(product).protocol
$(product).protocol: editor.$(product)
	rsync $(call c-source-dir-to-binary-dir,$(r.ui_plugin_remoteanim.base),editor)/inc/protocol $(DROW_RENDER_BIN) \
			--include "*.h" -r -d
