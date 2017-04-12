product:=testenv.gdpp_app
$(product).type:=lib
$(product).buildfor:=dev
$(product).product.c.includes:=include
$(product).depends:=gtest loki gdpp_app \
                    testenv.cpepp_utils testenv.gd_app
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
