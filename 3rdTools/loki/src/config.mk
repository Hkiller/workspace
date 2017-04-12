product:=loki
$(product).type:=lib
$(product).depends:=
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)
$(product).product.c.includes:=3rdTools/loki/include

$(eval $(call product-def,$(product)))