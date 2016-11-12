product:=testenv.gd_dr_dm
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=gd_dr_dm \
                    testenv.utils testenv.gd_utils testenv.gd_dr_store
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
