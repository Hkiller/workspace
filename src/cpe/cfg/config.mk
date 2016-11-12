product:=cpe_cfg
$(product).type:=lib
$(product).depends:=cpe_utils cpe_xcalc cpe_dr cpe_vfs cpe_zip yaml yajl
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
