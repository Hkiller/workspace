product:=gd_app_attr
$(product).type:=lib
$(product).depends:=cpe_dr cpe_xcalc gd_app
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product)))
