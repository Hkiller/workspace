product:=usf_bpg_pkg
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils cpe_dr_data_json gd_dr_cvt cpe_dr_data_cfg \
                    gd_app gd_dr_store zlib
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(product).cpe-dr.modules:=base base-use
$(product).cpe-dr.base.generate:=h c
$(product).cpe-dr.base.source:=$(wildcard $(CPDE_ROOT)/protocol/base/*.xml)
$(product).cpe-dr.base.h.output:=protocol/base
$(product).cpe-dr.base.c.output:=protocol/base/base_package.c
$(product).cpe-dr.base.c.arg-name:=g_metalib_base_package

$(product).cpe-dr.base-use.generate:=bin
$(product).cpe-dr.base-use.source:=$(wildcard $(CPDE_ROOT)/protocol/base/*.xml)
$(product).cpe-dr.base-use.bin.output:=meta/basepkg.dr

$(eval $(call product-def,$(product)))
