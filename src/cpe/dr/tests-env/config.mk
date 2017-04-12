product:=testenv.cpe_dr
$(product).type:=lib
$(product).buildfor:=dev
$(product).product.c.includes:=include
$(product).depends:=gtest cpe_dr cpe_dr_meta_inout testenv.utils
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
