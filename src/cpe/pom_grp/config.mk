product:=cpe_pom_grp
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_dr cpe_dr_data_cfg cpe_pom
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
