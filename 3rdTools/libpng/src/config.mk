product:=png
$(product).type:=lib
$(product).depends:=zlib
$(product).version:=1.5.7
$(product).product.c.includes:=3rdTools/libpng/include

$(product).c.sources := $(wildcard $(product-base)/*.c)
$(product).product.c.libraries:=
$(product).c.env-includes:=3rdTools/libpng/src
$(product).c.flags.cpp:= -DHAVE_CONFIG_H -DPNG_CONFIGURE_LIBPNG -Wno-format -Wno-unused -O2
$(product).c.flags.ld:=

$(eval $(call product-def,$(product)))
