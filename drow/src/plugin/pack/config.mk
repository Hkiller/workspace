product:=ui_plugin_pack
$(product).type:=lib
$(product).depends:=ui_model ui_cache ui_model_ed ui_plugin_particle
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
