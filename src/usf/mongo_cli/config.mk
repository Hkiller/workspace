product:=usf_mongo_cli
$(product).type:=cpe-dr lib
$(product).depends:=usf_mongo_driver usf_logic_use
$(product).product.c.flags.ld:=
$(product).product.c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=mongo_cli
$(product).cpe-dr.mongo_cli.generate:=h c
$(product).cpe-dr.mongo_cli.source:=$(wildcard $(product-base)/*.xml)
$(product).cpe-dr.mongo_cli.h.output:=protocol/mongo_cli
$(product).cpe-dr.mongo_cli.c.output:=protocol/mongo_cli/mongo_cli_all.c
$(product).cpe-dr.mongo_cli.c.arg-name:=g_metalib_mongo_cli

$(eval $(call product-def,$(product)))
