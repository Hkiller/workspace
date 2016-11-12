product:=cpe_utils_openssl
$(product).type:=lib
$(product).depends:=openssl cpe_utils
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
