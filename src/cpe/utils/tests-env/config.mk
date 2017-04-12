product:=testenv.utils
$(product).type:=lib
$(product).buildfor:=dev
$(product).product.c.includes:=include
$(product).depends:=gtest loki cpe_utils
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
