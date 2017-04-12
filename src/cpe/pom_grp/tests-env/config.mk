product:=testenv.cpe_pom_grp
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.cpe_cfg testenv.cpe_dr cpe_pom_grp
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
