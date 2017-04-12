product:=usf_bpg_tool
$(product).type:=progn 
$(product).depends:=argtable2 \
                    cpe_dr_data_json \
                    gd_app \
                    gd_net \
                    usf_logic \
                    usf_bpg_pkg \
                    usf_bpg_net \
                    usf_bpg_use
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.flags.ld:=-rdynamic \
                       $(foreach m,\
                            app_net_runner \
                            dr_store_manage \
                            dr_store_loader \
                            logic_manage \
                            bpg_metalib \
                            bpg_pkg_manage \
                            bpg_net_client \
                            , -u _$m_app_init -u _$m_app_fini)
$(eval $(call product-def,$(product),tools))

USF_BPG_TOOL=$(CPDE_OUTPUT_ROOT)/$(r.usf_bpg_tool.tools.product)