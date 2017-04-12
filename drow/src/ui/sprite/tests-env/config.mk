product:=testenv.ui_sprite
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.cpe_cfg testenv.gd_app ui_sprite
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
