#$(call gtest-def,pruduct,depends,append-srcs)
define gtest-def

$(eval product:=gtest.$1)
$(eval $(product).type:=progn run)
$(eval $(product).buildfor:=dev)
$(eval $(product).depends:=testenv.utils $1 $2)
$(eval $(product).product.c.includes:=include)
$(eval $(product).c.sources := $(wildcard $(product-base)/*.cpp)\
                               $(wildcard $(product-base)/*.c) \
                               $3 \
                               )
$(eval $(product).c.flags.ld+=$$(r.$1.c.flags.ld))
$(eval $(product).c.flags.lan.all+=-Wno-unused-value)
$(eval $(product).c.output-includes=..)
$(eval $(product).run.path:=$(product-base))
$(eval $1.ut:=$(product))

$(eval $(call product-def,$(product)))

endef
