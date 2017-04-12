product:=cpe_timer
$(product).type:=lib
$(product).depends:=cpe_tl
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
#$(product).c.flags.cpp:=-DCPE_TIMER_DEBUG

$(eval $(call product-def,$(product)))
