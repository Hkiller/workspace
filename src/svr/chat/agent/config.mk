product:=chat_agent
$(product).type:=cpe-dr lib
$(product).depends:=cpepp_cfg
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:=$(product-base)/symbols.def
$(product).product.c.flags.ld:=-rdynamic
$(product).product.c.output-includes:=share
$(product).include:=

$(product).cpe-dr.modules:=pro
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../svr/pro/cli/, svr_chat_pro.xml)
$(product).cpe-dr.pro.h.output:=share/protocol/svr/chat
$(product).cpe-dr.pro.h.with-traits:=svr_chat_meta_traits.cpp
$(product).cpe-dr.pro.c.output:=share/protocol/svr/chat/svr_chat_metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_chat_pro

$(eval $(call product-def,$(product)))
