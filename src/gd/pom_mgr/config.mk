product:=gd_pom_mgr
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_pom_grp
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
