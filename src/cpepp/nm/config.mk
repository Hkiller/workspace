product:=cpepp_nm
$(product).type:=lib
$(product).depends:=cpe_nm
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
