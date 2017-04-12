product:=ui_plugin_editor
$(product).type:=lib
$(product).depends:=ui_plugin_layout
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-dir+=$(product-base)/android/src

#flex
$(product).flex.c.sources:=$(wildcard $(product-base)/flex/*.c)

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
