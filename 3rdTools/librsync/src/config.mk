product:=librsync
$(product).type:=lib
$(product).depends:=
$(product).version:=2.0.1
$(product).product.c.includes:=3rdTools/librsync/src

$(product).c.sources := $(wildcard $(product-base)/*.c)
$(product).product.c.libraries:=
$(product).c.env-includes:=3rdTools/librsync/src
$(product).c.flags.cpp:=
$(product).c.flags.ld:=

$(eval $(call product-def,$(product)))
