product:=appsvr_apple_purchase
$(product).type:=lib
$(product).depends:=appsvr_payment
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def


#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm)
$(product).ios.product.c.frameworks:=StoreKit

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
