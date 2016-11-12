product:=set_share
$(product).type:=cpe-dr lib
$(product).depends:=cpe_pal cpe_utils cpe_dr_data_pbuf
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.flags.ld:=-rdynamic
$(product).product.c.output-includes:=share
$(product).product.include:=share

$(product).cpe-dr.modules:=data
#编译data协议定义
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/, set_share_pkg.xml set_share_chanel.xml)
$(product).cpe-dr.data.h.output:=share/protocol/svr/set/
$(product).cpe-dr.data.c.output:=share/protocol/svr/set/metalib.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_svr_set_share

$(eval $(call product-def,$(product)))
