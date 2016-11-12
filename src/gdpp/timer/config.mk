product:=gdpp_timer
$(product).type:=lib
$(product).depends:=gd_timer gdpp_app
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
