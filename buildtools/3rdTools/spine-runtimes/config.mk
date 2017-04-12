product:=spine-c
$(product).type:=lib
$(product).c.libraries:=
$(product).c.sources := $(wildcard $(CPDE_ROOT)/3rdTools/spine-runtimes/spine-c/src/spine/*.c)
$(product).c.flags.cpp:=
$(product).product.c.includes:=3rdTools/spine-runtimes/spine-c/include

$(eval $(call product-def,$(product)))
