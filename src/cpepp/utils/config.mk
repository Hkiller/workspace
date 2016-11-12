product:=cpepp_utils
$(product).type:=lib
$(product).depends:=cpe_utils
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
