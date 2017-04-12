product:=mongoc
$(product).type:=lib
$(product).depends:=libbson
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/mongoc/*.c)
$(product).product.c.includes:=3rdTools/mongoc/mongoc
$(product).product.c.env-includes:=3rdTools/mongoc/mongoc
$(product).c.flags.cpp:=-DMONGOC_COMPILATION

$(eval $(call product-def,$(product)))
