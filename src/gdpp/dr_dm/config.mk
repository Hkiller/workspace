product:=gdpp_dr_dm
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app gd_dr_dm
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
