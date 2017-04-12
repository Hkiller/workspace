product:=pcre2
$(product).type:=lib
$(product).product.c.includes:=3rdTools/pcre2/include
$(product).c.sources:=$(addprefix $(product-base)/src/, \
                         pcre2_auto_possess.c \
                         pcre2_chartables.c \
                         pcre2_compile.c \
                         pcre2_config.c \
                         pcre2_context.c \
                         pcre2_dfa_match.c \
                         pcre2_error.c \
                         pcre2_find_bracket.c  \
                         pcre2_jit_compile.c \
                         pcre2_maketables.c \
                         pcre2_match.c \
                         pcre2_match_data.c \
                         pcre2_newline.c \
                         pcre2_ord2utf.c \
                         pcre2_pattern_info.c \
                         pcre2_serialize.c \
                         pcre2_string_utils.c \
                         pcre2_study.c \
                         pcre2_substitute.c \
                         pcre2_substring.c \
                         pcre2_tables.c \
                         pcre2_ucd.c \
                         pcre2_valid_utf.c \
                         pcre2_xclass.c \
                       )
$(product).c.flags.cpp:=-DPCRE2_CODE_UNIT_WIDTH=8 -DHAVE_CONFIG_H
$(product).c.flags.c:= -O3
$(product).c.flags.ld:=
$(product).product.c.defs:=PCRE2_CODE_UNIT_WIDTH=8

$(eval $(call product-def,$(product)))
