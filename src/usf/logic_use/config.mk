product:=usf_logic_use
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils usf_logic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=logic_use
$(product).cpe-dr.logic_use.generate:=h c
$(product).cpe-dr.logic_use.source:=$(wildcard $(product-base)/*.xml)
$(product).cpe-dr.logic_use.h.output:=protocol/logic_use
$(product).cpe-dr.logic_use.c.output:=protocol/logic_use/logic_use_all.c
$(product).cpe-dr.logic_use.c.arg-name:=g_metalib_logic_use

$(eval $(call product-def,$(product),tools))
