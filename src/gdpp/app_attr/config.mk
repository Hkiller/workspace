product:=gdpp_app_attr
$(product).type:=lib
$(product).depends:=gdpp_app gd_app_attr
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
