product:=testenv.usfpp_logic
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.usf_logic \
                    usfpp_logic \
                    gmock
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
