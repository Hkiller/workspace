product:=testenv.usf_bpg_pkg
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.cpe_cfg \
                    testenv.gd_app \
                    testenv.gd_dr_store \
                    usf_bpg_pkg
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
