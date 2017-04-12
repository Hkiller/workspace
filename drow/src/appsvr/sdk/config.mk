product:=appsvr_sdk
$(product).type:=cpe-dr lib
$(product).depends:=plugin_app_env
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../../pro/appsvr_sdk/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/appsvr/sdk
$(product).cpe-dr.data.h.with-traits:=traits_appsvr_sdk.cpp
$(product).cpe-dr.data.c.output:=metalib_appsvr_sdk.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_appsvr_sdk

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
