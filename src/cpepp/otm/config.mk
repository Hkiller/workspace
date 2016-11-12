product:=cpepp_otm
$(product).type:=lib
$(product).depends:=cpe_otm
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
