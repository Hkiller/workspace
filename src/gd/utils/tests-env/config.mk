product:=testenv.gd_utils
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.gd_app gd_utils 
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
