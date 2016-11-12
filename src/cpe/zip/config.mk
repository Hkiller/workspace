product:=cpe_zip
$(product).type:=lib
$(product).depends:=zlib
$(product).product.c.includes:=include
$(product).libraries:=
$(product).c.sources:=$(addprefix $(product-base)/, \
                          ioapi.c \
                          unzip.c \
                          zip.c \
                          zip_file.c \
                          zip_dir.c \
                       )

$(eval $(call product-def,$(product)))
