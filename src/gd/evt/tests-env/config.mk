product:=testenv.gd_evt
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.gd_dr_store gd_evt 
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
