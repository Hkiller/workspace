product:=svrpp_center_agent
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app center_agent
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
