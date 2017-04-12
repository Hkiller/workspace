product:=render_utils
$(product).type:=lib
$(product).depends:=cpe_utils
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.includes:=drow/include

$(product).product.c.includes:=drow/include

#tools
$(product).tools.product.c.defs:=IN_EXPORT
$(product).tools.product.c.includes:=3rdTools/gles/dummy
$(product).tools.c.flags.cpp:=

#flex
$(product).flex.depends:=

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
