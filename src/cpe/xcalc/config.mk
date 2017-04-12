product:=cpe_xcalc
$(product).type:=lib
$(product).depends:=pcre2 cpe_utils
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
