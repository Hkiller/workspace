product:=svrpp_conn_net_logic
$(product).type:=lib
$(product).depends:=cpepp_utils conn_net_logic usfpp_logic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
