product:=gtest
$(product).type:=lib
$(product).buildfor:=dev
$(product).product.c.includes:=3rdTools/gtest/include
$(product).c.sources:=$(wildcard $(product-base)/*.cc)
$(product).c.includes:=3rdTools/gtest
$(product).product.c.libraries:=pthread
$(product).mac.product.c.defs:=GTEST_USE_OWN_TR1_TUPLE
$(product).ios.product.c.defs:=GTEST_USE_OWN_TR1_TUPLE

$(eval $(call product-def,$(product)))
