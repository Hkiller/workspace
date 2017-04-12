product:=freetype
$(product).type:=lib
$(product).depends:=zlib bz2
$(product).product.c.includes:=3rdTools/freetype/include 3rdTools/freetype/include/freetype
$(product).version:=2.4.9
$(product).c.includes:=
$(product).c.sources:=$(addprefix $(product-base)/, \
                         base/ftsystem.c \
                         base/ftdebug.c \
                         base/ftinit.c \
                         base/ftbase.c \
                         base/ftbbox.c \
                         base/ftbdf.c \
                         base/ftbitmap.c \
                         base/ftcid.c \
                         base/ftfstype.c \
                         base/ftgasp.c \
                         base/ftglyph.c \
                         base/ftgxval.c \
                         base/ftlcdfil.c \
                         base/ftmm.c \
                         base/ftotval.c \
                         base/ftpatent.c \
                         base/ftpfr.c \
                         base/ftstroke.c \
                         base/ftsynth.c \
                         base/fttype1.c \
                         base/ftwinfnt.c \
                         base/ftxf86.c \
                         truetype/truetype.c \
                         type1/type1.c \
                         cff/cff.c \
                         cid/type1cid.c \
                         pfr/pfr.c \
                         type42/type42.c \
                         winfonts/winfnt.c \
                         pcf/pcf.c \
                         bdf/bdf.c \
                         sfnt/sfnt.c \
                         autofit/autofit.c \
                         pshinter/pshinter.c \
                         raster/raster.c \
                         smooth/smooth.c \
                         cache/ftcache.c \
                         gzip/ftgzip.c \
                         lzw/ftlzw.c \
                         psaux/psaux.c \
                         psnames/psnames.c \
                       )

$(product).c.flags.cpp:=-DFT_CONFIG_OPTION_SYSTEM_ZLIB \
                        -DDARWIN_NO_CARBON \
                        '-DFT_CONFIG_CONFIG_H="freetype/config/ftconfig.h"' \
                        '-DFT_CONFIG_MODULES_H="freetype/config/ftmodule.h"' \
                        -DFT2_BUILD_LIBRARY
$(product).c.flags.c:=-O2
$(product).c.flags.ld:=
$(product).product.c.defs:=

$(eval $(call product-def,$(product)))
