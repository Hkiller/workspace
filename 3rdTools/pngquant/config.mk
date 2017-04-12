product:=pngquant
$(product).type:=lib
$(product).depends:=png
$(product).version:=1.0
$(product).product.c.includes:=3rdTools/pngquant
$(product).c.sources := $(product-base)/rwpng.c
$(product).product.c.libraries:=
$(product).c.flags.cpp:= -O3 -Wall -funroll-loops -fomit-frame-pointer
$(product).c.flags.ld:=

$(eval $(call product-def,$(product)))
