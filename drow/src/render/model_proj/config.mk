product:=ui_model_proj
$(product).type:=lib 
$(product).depends:=cpe_utils_xml ui_model 
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
