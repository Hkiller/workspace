product:=uhub_svr_lib
$(product).type:=lib 
$(product).depends:=cpe_cfg cpe_dr cpe_dr_data_cfg cpe_dr_data_pbuf cpe_tl cpe_dp cpe_nm \
                    gd_net gd_app gd_log gd_dr_store usf_mongo_use \
                    set_stub

$(product).c.sources:=$(filter-out %/main.c,$(wildcard $(product-base)/*.c))

$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base)))

$(product).cpe-dr.modules:=pro pro-use

$(eval $(call product-def,$(product)))

product:=uhub_svr
$(product).type:=progn
$(product).depends:=uhub_svr_lib argtable2
$(product).c.sources:=$(product-base)/main.c
$(product).c.flags.ld:=-lm -rdynamic
$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product)))
