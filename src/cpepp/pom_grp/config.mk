product:=cpepp_pom_grp
$(product).type:=lib
$(product).depends:=cpepp_utils cpe_pom_grp
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
