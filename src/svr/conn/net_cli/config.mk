product:=conn_net_cli
$(product).type:=cpe-dr lib
$(product).depends:=cpe_net cpe_fsm gd_app
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=pro
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../svr/pro/, net/svr_conn_net.xml)
$(product).cpe-dr.pro.align:=1
$(product).cpe-dr.pro.h.output:=protocol/svr/conn
$(product).cpe-dr.pro.c.output:=protocol/svr/conn/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_conn_pro

$(eval $(call product-def,$(product)))
