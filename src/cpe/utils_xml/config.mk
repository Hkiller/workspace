product:=cpe_utils_xml
$(product).type:=lib
$(product).depends:=xml2 cpe_utils
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
