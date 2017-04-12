product:=gif
$(product).type:=lib
#$(product).version:=1.5.7
$(product).product.c.includes:=3rdTools/libgif/include

$(product).c.sources := $(wildcard $(product-base)/src/gif_*.c) \
                        $(product-base)/src/dgif_lib.c \
                        $(product-base)/src/gifalloc.c

$(product).product.c.libraries:=
$(product).c.flags.cpp:= -DHAVE_CONFIG_H
$(product).c.flags.ld:=

$(eval $(call product-def,$(product)))
