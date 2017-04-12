product:=ui_plugin_swf
$(product).type:=lib
$(product).depends:=ui_runtime gameswf
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)
$(product).c.export-symbols:=$(product-base)/symbols.def

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
