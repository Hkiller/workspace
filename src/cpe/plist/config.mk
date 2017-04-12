product:=cpe_plist
$(product).type:=lib
$(product).depends:=xml2 cpe_cfg
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
