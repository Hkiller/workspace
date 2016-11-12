product:=gdpp_net_trans
$(product).type:=lib
$(product).depends:=gd_net_trans gdpp_app
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
