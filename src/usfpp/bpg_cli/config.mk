product:=usfpp_bpg_cli
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app usf_bpg_cli usfpp_bpg_pkg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
