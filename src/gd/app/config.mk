product:=gd_app
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_tl cpe_dp cpe_nm cpe_net cpe_vfs cpe_statistics
$(product).product.c.env-libraries:=dl
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.libraries:=$($(product).c.libraries)
$(product).product.c.flags.ld:=$($(product).c.flags.ld)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(product).linux32.product.c.libraries+=dl
$(product).linux64.product.c.libraries+=dl

gd-app-multi-thread?=0
ifeq ($(gd-app-multi-thread),1)
$(product).c.flags.cpp+=$(if $(filter 1,$(gd-app-multi-thread)), -DGD_APP_MULTI_THREAD )

$(product).mac.product.c.libraries+=pthread
$(product).linux32.product.c.libraries+=pthread
$(product).linux64.product.c.libraries+=pthread
$(product).ios.product.c.libraries+=pthread

endif

$(eval $(call product-def,$(product)))
