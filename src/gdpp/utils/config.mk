product:=gdpp_utils
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app gd_utils
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
