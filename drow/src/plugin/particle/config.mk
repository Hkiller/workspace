product:=ui_plugin_particle
$(product).type:=lib
$(product).depends:= ui_runtime
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
