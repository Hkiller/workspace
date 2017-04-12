product:=testenv.cpe_dp
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils cpe_dp
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
