product:=bz2

$(product).type:=virtual
$(product).mac.product.c.libraries+=bz2
$(product).ios.product.c.libraries+=bz2

$(eval $(call product-def,$(product)))
