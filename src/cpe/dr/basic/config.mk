product:=cpe_dr
$(product).type:=lib
$(product).depends:=cpe_utils
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.flags.cpp:=-Wno-format

$(eval $(call product-def,$(product)))
