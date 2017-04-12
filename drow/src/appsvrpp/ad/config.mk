product:=appsvrpp_ad
$(product).type:=lib
$(product).depends:=appsvr_ad
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
