product:=harfbuzz
$(product).type:=lib
$(product).depends:=freetype
$(product).product.c.includes:=3rdTools/harfbuzz/include
$(product).version:=1.2.5
$(product).c.includes:=3rdTools/harfbuzz/src/hb-ucdn
$(product).c.sources:=$(addprefix $(product-base)/src/, \
	hb-blob.cc \
	hb-buffer-serialize.cc \
	hb-buffer.cc \
	hb-common.cc \
	hb-ft.cc \
    hb-fallback-shape.cc \
	hb-face.cc \
	hb-font.cc \
	hb-set.cc \
	hb-shape.cc \
	hb-shape-plan.cc \
	hb-shaper.cc \
	hb-unicode.cc \
	hb-warning.cc \
    hb-ucdn.cc \
    hb-ucdn/ucdn.c \
)

$(product).c.flags.cpp:=-DHAVE_CONFIG_H
$(product).c.flags.c:=-O2
$(product).c.flags.ld:=
$(product).c.env-includes:=3rdTools/harfbuzz/src
$(product).product.c.defs:=

$(eval $(call product-def,$(product)))
