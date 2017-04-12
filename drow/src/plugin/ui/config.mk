product:=ui_plugin_ui
$(product).type:=lib
$(product).depends:=ui_model cpe_xcalc ui_plugin_layout ui_plugin_editor ui_plugin_package ui_plugin_basicanim
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
