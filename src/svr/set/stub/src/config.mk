product:=set_stub
$(product).type:=cpe-dr lib
$(product).depends:=cpe_cfg cpe_fsm set_share argtable2 cpe_dp
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.flags.ld:=-rdynamic
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=pro
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../../svr/pro/, cli/svr_set_data.xml cli/svr_set_pro.xml)
$(product).cpe-dr.pro.h.output:=protocol/svr/set
$(product).cpe-dr.pro.c.output:=protocol/svr/set/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_set_pro

$(eval $(call product-def,$(product)))
