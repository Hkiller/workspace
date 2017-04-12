product:=ui_model_manip
$(product).type:=lib 
$(product).depends:=cpe_plist ui_model ui_model_ed ui_cache
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
