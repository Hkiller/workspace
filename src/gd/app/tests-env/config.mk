product:=testenv.gd_app
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils gd_app
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
