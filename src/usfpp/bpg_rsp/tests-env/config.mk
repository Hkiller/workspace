product:=testenv.usfpp_bpg_rsp
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.usf_bpg_rsp
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
