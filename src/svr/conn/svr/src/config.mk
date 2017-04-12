product:=conn_svr_lib
$(product).type:=cpe-dr lib 
$(product).depends:=ev cpe_cfg cpe_dr cpe_dr_data_cfg cpe_dr_data_pbuf cpe_tl cpe_dp cpe_nm \
                    gd_net gd_app  gd_dr_cvt gd_log gd_dr_store gd_timer \
                    set_stub

$(product).c.sources:=$(filter-out %/main.c,$(wildcard $(product-base)/*.c))

$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base)))
$(product).product.c.flags.ld:=-rdynamic

$(product).cpe-dr.modules:=pro pro-use net
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../pro/, \
                                   cli/svr_conn_data.xml cli/svr_conn_pro.xml \
                                   svr/svr_conn_meta.xml svr/svr_conn_internal.xml)
$(product).cpe-dr.pro.h.output:=protocol/svr/conn
$(product).cpe-dr.pro.c.output:=protocol/svr/conn/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_conn_pro

$(product).cpe-dr.pro-use.generate:=bin
$(product).cpe-dr.pro-use.source:=$(addprefix $(product-base)/../pro/, \
                                   cli/svr_conn_data.xml cli/svr_conn_pro.xml)
$(product).cpe-dr.pro-use.bin.output:=meta/conn_svr.dr

$(product).cpe-dr.net.generate:=h
$(product).cpe-dr.net.source:=$(addprefix $(product-base)/../pro/, net/svr_conn_net.xml)
$(product).cpe-dr.net.align:=1
$(product).cpe-dr.net.h.output:=protocol/svr/conn

$(eval $(call product-def,$(product)))

product:=conn_svr
$(product).type:=progn
$(product).depends:=conn_svr_lib argtable2
$(product).c.sources:=$(product-base)/main.c
$(product).c.flags.ld:=-lm -rdynamic
$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product)))
