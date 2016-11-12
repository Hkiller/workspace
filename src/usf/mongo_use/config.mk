product:=usf_mongo_use
$(product).type:=cpe-dr lib
$(product).depends:=usf_mongo_cli
$(product).product.c.flags.ld:=
$(product).product.c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=mongo-use
$(product).cpe-dr.mongo-use.generate:=h c
$(product).cpe-dr.mongo-use.source:=$(product-base)/mongo_use.xml
$(product).cpe-dr.mongo-use.h.output:=protocol/mongo_use
$(product).cpe-dr.mongo-use.c.output:=protocol/mongo_use/mongo_use_data.c
$(product).cpe-dr.mongo-use.c.arg-name:=g_metalib_mongo_use_data

$(eval $(call product-def,$(product)))
