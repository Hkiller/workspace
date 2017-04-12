product:=testenv.render_utils
$(product).type:=lib
$(product).buildfor:=dev
$(product).product.c.includes:=include
$(product).depends:=render_utils testenv.utils
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
