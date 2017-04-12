product:=vorbis
$(product).type:=lib
$(product).depends:=ogg
$(product).c.libraries:=
$(product).version:=1.3.4
$(product).c.sources:=$(addprefix $(product-base)/lib/, \
                           mdct.c \
                           smallft.c \
                           block.c \
                           envelope.c \
                           window.c \
                           lsp.c \
                           lpc.c \
                           analysis.c \
                           synthesis.c \
                           psy.c \
                           info.c \
                           floor1.c \
                           floor0.c \
                           res0.c \
                           mapping0.c \
                           registry.c \
                           codebook.c \
                           sharedbook.c \
                           lookup.c \
                           bitrate.c \
                           vorbisfile.c \
                         )

$(product).product.c.includes:=3rdTools/libvorbis/include
$(product).c.env-includes:=3rdTools/libvorbis/lib
$(product).c.includes:=$(product-base)/lib
$(product).c.flags.cpp:=-DHAVE_CONFIG_H -Wno-uninitialized

$(eval $(call product-def,$(product)))
