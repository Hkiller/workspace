product:=ui_model
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils cpe_dr cpe_dr_data_basic ui_cache render_utils
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def

$(product).product.c.output-includes:=inc

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/model/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/render/model
$(product).cpe-dr.data.c.output:=metalib_render_model.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_render_model

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))

.POHEY: $(product).protocol
drow_render.export: $(product).protocol
$(product).protocol: editor.$(product)
	rsync $(call c-source-dir-to-binary-dir,$(r.ui_model.base),editor)/inc/protocol $(DROW_RENDER_BIN) --include "*.h" -r -d
