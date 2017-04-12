product:=usfpp_bpg_pkg
$(product).type:=lib
$(product).depends:=cpepp_utils cpepp_dr gdpp_app usf_bpg_pkg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
