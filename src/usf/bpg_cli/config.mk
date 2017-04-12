product:=usf_bpg_cli
$(product).type:= cpe-dr lib
$(product).depends:=cpe_utils usf_bpg_pkg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(product).cpe-dr.modules:=pkg_info
$(product).cpe-dr.pkg_info.generate:=h c
$(product).cpe-dr.pkg_info.source:=$(product-base)/bpg_cli_pkg_info.xml
$(product).cpe-dr.pkg_info.h.output:=protocol/bpg_cli
$(product).cpe-dr.pkg_info.c.output:=protocol/bpg_cli/bpg_cli_pkg_info.c
$(product).cpe-dr.pkg_info.c.arg-name:=g_metalib_bpg_cli_pkg_info

$(eval $(call product-def,$(product),tools))
