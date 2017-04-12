product:=center_agent
$(product).type:=cpe-dr lib
$(product).depends:=cpe_cfg cpe_fsm
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.flags.ld:=-rdynamic
$(product).product.c.output-includes:=share
$(product).product.include:=share

$(product).cpe-dr.modules:=pro
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../../svr/pro/cli/, svr_center_data.xml svr_center_pro.xml)
$(product).cpe-dr.pro.h.output:=share/protocol/svr/center
$(product).cpe-dr.pro.c.output:=share/protocol/svr/center/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_center_pro

$(eval $(call product-def,$(product)))
