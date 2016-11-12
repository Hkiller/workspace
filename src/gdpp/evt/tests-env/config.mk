product:=testenv.gdpp_evt
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.gd_evt
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
