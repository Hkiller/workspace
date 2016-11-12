product:=payment_deliver_svr_lib
$(product).type:=lib 
$(product).depends:=ebb cpe_cfg cpe_dr cpe_dr_data_http_args cpe_utils_openssl cpe_dr_data_json cpe_tl cpe_dp cpe_nm \
                    gd_net gd_app gd_log gd_dr_store gd_timer \
                    set_stub payment_agent

$(product).c.sources:=$(filter-out %/main.c,$(wildcard $(product-base)/*.c))

$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base)))

$(eval $(call product-def,$(product)))

product:=payment_deliver_svr
$(product).type:=progn
$(product).depends:=payment_deliver_svr_lib argtable2
$(product).c.sources:=$(product-base)/main.c
$(product).c.flags.ld:=-lm -rdynamic
$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product)))
