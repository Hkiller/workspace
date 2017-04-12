product:=zlib

$(product).type:=virtual
$(product).product.c.libraries+=z

$(product).emscripten.product.c.includes:=3rdTools/zlib/emscripten/include
$(product).emscripten.product.c.ldpathes:=3rdTools/zlib/emscripten/lib

$(eval $(call product-def,$(product)))
