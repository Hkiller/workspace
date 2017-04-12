product:=iconv

$(product).type:=virtual

$(product).cygwin.product.c.libraries:=iconv

$(product).mac.product.c.libraries:=iconv

$(eval $(call product-def,$(product)))
