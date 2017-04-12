product:=libbson
$(product).type:=lib
$(product).depends:=yajl
$(product).c.libraries:=
$(product).version:=
$(product).c.sources := $(filter-out %/bson-json.c %/bson-decimal128.c, $(wildcard $(product-base)/src/bson/*.c))

$(product).product.c.includes:=3rdTools/libbson/src/bson
$(product).product.c.env-includes:=3rdTools/libbson/src/bson
$(product).c.flags.cpp:=-DBSON_COMPILATION
$(product).c.flags.ld:=

$(eval $(call product-def,$(product)))
