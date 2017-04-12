product:=ui_plugin_mask_manip
$(product).type:=lib
$(product).depends:=ui_cache ui_plugin_mask ui_plugin_tiledmap ui_plugin_package_manip
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
