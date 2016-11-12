product:=testenv.set_stub
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.gd_app testenv.cpe_cfg
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)
$(product).c.output-includes=..

$(eval $(call product-def,$(product)))
