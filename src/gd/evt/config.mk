product:=gd_evt
$(product).type:=lib
$(product).depends:=cpe_tl cpe_dr cpe_dr_data_basic cpe_dr_data_json \
                    gd_app gd_dr_store
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
