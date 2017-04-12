product:=ui_plugin_spine
$(product).type:=cpe-dr lib
$(product).depends:=ui_runtime spine-c
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def
$(product).product.c.output-includes:=inc

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/spine/,\
                                   $(shell cat $(product-base)/protocol.def) \
                                 )
$(product).cpe-dr.data.h.output:=inc/protocol/plugin/spine
$(product).cpe-dr.data.c.output:=metalib_plugin_spine.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_plugin_spine

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
