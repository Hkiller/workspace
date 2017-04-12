product:=ui_plugin_scrollmap_manip
$(product).type:=lib
$(product).depends:=ui_plugin_scrollmap ui_plugin_tiledmap ui_model_ed ui_plugin_package_manip
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
