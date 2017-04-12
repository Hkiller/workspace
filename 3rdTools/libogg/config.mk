product:=ogg
$(product).type:=lib
$(product).c.libraries:=
$(product).version:=1.3.2
$(product).c.sources:=$(wildcard $(product-base)/src/*.c)

$(product).product.c.includes:=3rdTools/libogg/include
$(product).c.env-includes:=3rdTools/libogg/src
$(product).c.flags.cpp:=-DHAVE_CONFIG_H

$(eval $(call product-def,$(product)))
