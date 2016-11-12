product:=cpe_fsm
$(product).type:=lib
$(product).depends:=cpe_utils
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
