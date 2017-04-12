product:=ui_plugin_render_s3d
$(product).type:=lib
$(product).depends:=ui_cache ui_runtime plugin_app_env
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).product.c.flags.ld:=-fllvm-llc-opt=-ascopt=-import \
                               -fllvm-llc-opt=-ascopt=$(product-base)/AGAL/AGAL.abc \
                               $(product-base)/AGAL/AGAL.o

$(product).flex.libraries:=$(product-base)/AGAL/AGAL.abc

#注册额
DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))

.POHEY: $(product).protocol
drow_render.export: $(product).protocol
