product:=svrpp_uhub_agent
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app uhub_agent
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
