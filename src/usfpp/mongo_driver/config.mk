product:=usfpp_mongo_driver
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app usf_mongo_driver
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
