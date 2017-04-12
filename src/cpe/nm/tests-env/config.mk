product:=testenv.cpe_nm
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils cpe_nm
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
