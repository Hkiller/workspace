product:=ebb
$(product).type:=ragel lib
$(product).depends:=ev
$(product).c.libraries:=
$(product).version:=4.0.4
$(product).c.sources := $(addprefix $(product-base)/, \
                          ebb.c \
                          rbtree.c \
                       )
$(product).product.c.includes:=3rdTools/libebb/include
$(product).c.flags.cpp:=-Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
$(product).c.flags.ld:=
$(product).ragel:=$(call def-ragel-to-c,$(product-base)/ebb_request_parser.rl)

$(eval $(call product-def,$(product)))
