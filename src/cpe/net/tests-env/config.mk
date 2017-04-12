product:=testenv.cpe_net
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils cpe_net
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
