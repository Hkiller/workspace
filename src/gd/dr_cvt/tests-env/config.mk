product:=testenv.gd_dr_cvt
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.gd_app gd_dr_cvt 
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
