product:=center_svr_lib
$(product).type:=cpe-dr lib 
$(product).depends:=cpe_cfg cpe_dr cpe_dr_meta_inout cpe_dr_data_json cpe_dr_data_pbuf cpe_aom cpe_dp cpe_nm \
                    gd_net gd_app gd_log gd_timer

$(product).c.sources:=$(filter-out %/main.c,$(wildcard $(product-base)/*.c))
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base)))
$(product).product.c.flags.ld:=-rdynamic

$(product).cpe-dr.modules:=pro
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../pro/, cli/svr_center_data.xml cli/svr_center_pro.xml svr/svr_center_internal.xml)
$(product).cpe-dr.pro.h.output:=protocol/svr/center
$(product).cpe-dr.pro.c.output:=protocol/svr/center/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_center_pro

$(eval $(call product-def,$(product)))

product:=center_svr
$(product).type:=progn
$(product).depends:=center_svr_lib argtable2
$(product).c.sources:=$(product-base)/main.c
$(product).c.flags.ld:=-lm -rdynamic
$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product)))
