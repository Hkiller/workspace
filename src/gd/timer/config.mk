product:=gd_timer
$(product).type:=lib
$(product).depends:=cpe_timer gd_app
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.flags.cpp:=-DGD_TIMER_DEBUG
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
