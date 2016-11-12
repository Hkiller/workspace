product:=cpepp_pom
$(product).type:=lib
$(product).depends:=cpepp_utils cpe_pom
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
