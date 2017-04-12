product:=payment_svr_lib
$(product).type:=cpe-dr lib 
$(product).depends:=cpe_cfg cpe_dr cpe_utils_openssl cpe_dr_data_cfg cpe_dr_data_pbuf cpe_tl cpe_dp cpe_nm \
                    gd_net gd_app gd_log gd_dr_store gd_net_trans usf_mongo_use \
                    set_logic apple_iap_agent

$(product).c.sources:=$(filter-out %/main.c,$(wildcard $(product-base)/*.c))

$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base)))

$(product).cpe-dr.modules:=pro pro-use
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../pro/, \
                                   cli/svr_payment_data.xml \
                                   cli/svr_payment_iapppay.xml \
                                   cli/svr_payment_qihoo.xml \
                                   cli/svr_payment_damai.xml \
                                   cli/svr_payment_notify.xml \
                                   cli/svr_payment_pro.xml \
                                   svr/svr_payment_internal.xml svr/svr_payment_meta.xml)
$(product).cpe-dr.pro.h.output:=protocol/svr/payment
$(product).cpe-dr.pro.c.output:=protocol/svr/payment/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_payment_pro


$(product).cpe-dr.pro-use.generate:=bin
$(product).cpe-dr.pro-use.source:=$(addprefix $(product-base)/../pro/, \
	                                   cli/svr_payment_data.xml \
                                       cli/svr_payment_iapppay.xml \
                                       cli/svr_payment_qihoo.xml \
                                       cli/svr_payment_damai.xml \
                                       cli/svr_payment_notify.xml \
                                       cli/svr_payment_pro.xml)
$(product).cpe-dr.pro-use.bin.output:=meta/payment_svr.dr

$(eval $(call product-def,$(product)))

product:=payment_svr
$(product).type:=progn
$(product).depends:=payment_svr_lib argtable2
$(product).c.sources:=$(product-base)/main.c
$(product).c.flags.ld:=-lm -rdynamic
$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product)))
