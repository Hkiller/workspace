product:=c-ares
$(product).type:=lib

$(product).c.sources:=$(filter-out %/acountry.c %/adig.c %/ahost.c, $(wildcard $(product-base)/*.c))
$(product).product.c.includes:=3rdTools/c-ares/include
$(product).product.c.env-includes:=3rdTools/c-ares/include
$(product).c.env-includes:=3rdTools/c-ares/src
$(product).c.flags.cpp:=-DHAVE_CONFIG_H 

$(eval $(call product-def,$(product)))
