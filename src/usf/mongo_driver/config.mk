product:=usf_mongo_driver
$(product).type:=lib
$(product).depends:=libbson openssl gd_app cpe_dr cpe_dr_data_bson cpe_fsm gd_net_dns
$(product).product.c.flags.ld:=
$(product).product.c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
