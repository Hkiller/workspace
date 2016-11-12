product:=testenv.usf_logic
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.cpe_cfg \
                    testenv.gd_app \
                    usf_logic
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
