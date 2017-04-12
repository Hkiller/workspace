product:=svrpp_set_stub
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app set_stub 
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
