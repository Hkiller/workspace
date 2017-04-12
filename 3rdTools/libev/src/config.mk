product:=ev
$(product).type:=lib
$(product).c.libraries:=
$(product).version:=4.0.4
$(product).c.sources := $(addprefix $(product-base)/, \
                          ev.c \
                          event.c \
                       )

$(product).product.c.includes:=3rdTools/libev/include
$(product).c.env-includes:=3rdTools/libev/src
$(product).c.flags.cpp:=-Wno-unused -Wno-parentheses
$(product).c.flags.ld:=-lm

$(eval $(call product-def,$(product)))
