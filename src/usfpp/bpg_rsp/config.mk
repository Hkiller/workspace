product:=usfpp_bpg_rsp
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app usfpp_logic usf_bpg_rsp
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
