product:=usfpp_logic
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app usf_logic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
