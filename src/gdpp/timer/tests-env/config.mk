product:=testenv.gdpp_timer
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.gd_timer
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
