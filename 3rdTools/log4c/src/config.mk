product:=log4c
$(product).type:=lib
$(product).product.c.includes:=3rdTools/log4c/include

$(product).c.sources:= $(addprefix $(product-base)/, \
                         rc.c \
                         init.c \
                         appender_type_stream.c \
                         appender_type_stream2.c \
                         appender_type_syslog.c \
                         appender_type_mmap.c \
                         layout_type_basic.c \
                         layout_type_dated.c \
                         layout_type_basic_r.c \
                         layout_type_dated_r.c \
                         version.c \
                         logging_event.c \
                         priority.c \
                         appender.c \
                         layout.c \
                         category.c \
                         appender_type_rollingfile.c \
                         rollingpolicy.c \
                         rollingpolicy_type_sizewin.c) \
                    $(addprefix $(product-base)/sd/, \
                         stack.c \
                         list.c \
                         malloc.c \
                         factory.c \
                         hash.c \
                         sprintf.c \
                         test.c \
                         sd_xplatform.c \
                         error.c \
                         domnode.c \
                         domnode-xml.c \
                         domnode-xml-parser.c \
                         domnode-xml-scanner.c )

$(product).c.flags.ld:=
$(product).c.includes:=3rdTools/log4c/src 3rdTools/log4c/include/log4c
$(product).c.env-includes:=3rdTools/log4c/src
$(product).c.flags.cpp:=-DHAVE_CONFIG_H -DLOG4C_RCPATH=\"\" -Wno-unused

$(eval $(call product-def,$(product)))
