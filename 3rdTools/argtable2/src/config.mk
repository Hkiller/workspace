product:=argtable2
$(product).type:=lib
$(product).c.libraries:=
$(product).c.sources := $(wildcard $(product-base)/*.c)
$(product).c.env-includes:=3rdTools/argtable2/src
$(product).c.flags.cpp:=-DHAVE_CONFIG_H -DNDEBUG
$(product).product.c.includes:=3rdTools/argtable2/include
$(product).version:=2.13

$(eval $(call product-def,$(product)))
