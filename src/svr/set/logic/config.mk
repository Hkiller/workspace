product:=set_logic
$(product).type:=cpe-dr lib
$(product).depends:=set_stub usf_logic
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(product-base)/set_logic_rsp_carry_info.xml \
                               $(product-base)/set_logic_sp_data.xml
$(product).cpe-dr.data.h.output:=protocol/set/logic
$(product).cpe-dr.data.c.output:=protocol/set/logic/data_metalib.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_set_logic_data_meta

$(eval $(call product-def,$(product)))
