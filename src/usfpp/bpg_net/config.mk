product:=usfpp_bpg_net
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app usf_bpg_net usfpp_bpg_pkg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
