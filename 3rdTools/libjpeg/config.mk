product:=jpeg
$(product).type:=lib
$(product).product.c.includes:=3rdTools/libjpeg
$(product).product.c.env-includes:=3rdTools/libjpeg

$(product).c.sources := $(addprefix $(product-base)/, \
                              jcapimin.c \
                              jcapistd.c \
                              jccoefct.c \
                              jccolor.c \
                              jcdctmgr.c \
                              jchuff.c \
                              jcinit.c \
                              jcmainct.c \
                              jcmarker.c \
                              jcmaster.c \
                              jcomapi.c \
                              jcparam.c \
                              jcprepct.c \
                              jcsample.c \
                              jctrans.c \
                              jdapimin.c \
                              jdapistd.c \
                              jdatadst.c \
                              jdatasrc.c \
                              jdcoefct.c \
                              jdcolor.c \
                              jddctmgr.c \
                              jdhuff.c \
                              jdinput.c \
                              jdmainct.c \
                              jdmarker.c \
                              jdmaster.c \
                              jdmerge.c \
                              jdpostct.c \
                              jdsample.c \
                              jdtrans.c \
                              jerror.c \
                              jfdctflt.c \
                              jfdctfst.c \
                              jfdctint.c \
                              jidctflt.c \
                              jidctfst.c \
                              jidctint.c \
                              jmemmgr.c \
                              jmemnobs.c \
                              jquant1.c \
                              jquant2.c \
                              jutils.c \
                              jcarith.c \
                              jdarith.c \
                              jaricom.c \
                       )

$(eval $(call product-def,$(product)))
