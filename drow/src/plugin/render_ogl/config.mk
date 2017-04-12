product:=ui_plugin_render_ogl
$(product).type:=lib
$(product).depends:=opengl ui_cache ui_runtime plugin_app_env
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.m)
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)

$(product).ios.product.c.frameworks:=

#注册额
DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))

.POHEY: $(product).protocol
drow_render.export: $(product).protocol
