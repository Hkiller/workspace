product:=set_svr_lib
$(product).type:=cpe-dr lib 
$(product).depends:=cpe_cfg cpe_dr cpe_dr_data_cfg cpe_dr_data_pbuf cpe_dr_data_json cpe_dp cpe_nm \
                    gd_net gd_app  gd_dr_store gd_log gd_timer set_share center_agent
$(product).c.sources:=$(filter-out %/main.c,$(wildcard $(product-base)/*.c))
$(product).product.c.output-includes:=inc

$(product).cpe-dr.modules:=pro
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../pro/, cli/svr_set_data.xml cli/svr_set_pro.xml svr/svr_set_internal.xml)
$(product).cpe-dr.pro.h.output:=inc/protocol/svr/set
$(product).cpe-dr.pro.c.output:=inc/protocol/svr/set/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_set_pro

$(eval $(call product-def,$(product)))

product:=set_svr
$(product).type:=progn
$(product).depends:=set_svr_lib argtable2
$(product).c.sources:=$(product-base)/main.c
$(product).c.flags.ld:=-lm -rdynamic
$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product)))
