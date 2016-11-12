product:=rank_g_svr_lib
$(product).type:=cpe-dr lib 
$(product).depends:=cpe_cfg cpe_dr cpe_dr_data_cfg cpe_dr_data_pbuf cpe_tl cpe_aom cpe_dp cpe_nm \
                    gd_app  gd_dr_store gd_timer gd_log \
                    set_stub set_logic usf_mongo_cli


$(product).c.sources:=$(filter-out %/main.c,$(wildcard $(product-base)/*.c))

$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base)))
$(product).product.c.flags.ld:=-rdynamic

$(product).cpe-dr.modules:=pro pro-use
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../pro/, \
                                   cli/svr_rank_g_pro.xml \
                                   svr/svr_rank_g_internal.xml)
$(product).cpe-dr.pro.h.output:=protocol/svr/rank_g
$(product).cpe-dr.pro.c.output:=protocol/svr/rank_g/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_rank_g_pro


$(product).cpe-dr.pro-use.generate:=bin
$(product).cpe-dr.pro-use.source:=$(addprefix $(product-base)/../pro/, \
                                   cli/svr_rank_g_pro.xml)
$(product).cpe-dr.pro-use.bin.output:=meta/rank_g_svr.dr

$(eval $(call product-def,$(product)))

product:=rank_g_svr
$(product).type:=progn
$(product).depends:=rank_g_svr_lib argtable2
$(product).c.sources:=$(product-base)/main.c
$(product).c.flags.ld:=-lm -rdynamic
$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product)))
