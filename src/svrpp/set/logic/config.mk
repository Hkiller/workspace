product:=svrpp_set_logic
$(product).type:=lib
$(product).depends:=cpepp_utils set_logic svrpp_set_stub usfpp_logic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
