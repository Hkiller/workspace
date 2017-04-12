product:=gd_net_trans
$(product).type:=lib
$(product).depends:=ev curl gd_app
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
