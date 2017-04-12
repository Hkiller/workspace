product:=gdpp_app
$(product).type:=lib
$(product).depends:=cpepp_utils cpepp_cfg gd_app cpepp_nm cpepp_dp
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
