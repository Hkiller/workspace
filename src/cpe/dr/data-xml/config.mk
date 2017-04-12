product:=cpe_dr_data_xml
$(product).type:=lib
$(product).depends:=cpe_dr cpe_utils xml2
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
