product:=gd_vnet
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils gd_app
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(product-base)/vnet_data.xml
$(product).cpe-dr.data.h.output:=protocol/vnet
$(product).cpe-dr.data.c.output:=protocol/vnet/data_package.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_vnet_metalib

$(eval $(call product-def,$(product)))



