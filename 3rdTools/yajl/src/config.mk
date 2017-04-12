product:=yajl
$(product).type:=lib
$(product).product.c.includes:=3rdTools/yajl/include
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.flags.cpp:=-DNDEBUG
$(product).c.flags.lan.c:= -O2 -Wno-conversion 
$(product).c.flags.ld:=

$(eval $(call product-def,$(product)))
