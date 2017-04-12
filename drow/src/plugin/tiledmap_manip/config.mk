product:=ui_plugin_tiledmap_manip
$(product).type:=lib
$(product).depends:=cpe_utils_xml cpe_dr_data_cfg ui_plugin_tiledmap ui_model_ed
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
