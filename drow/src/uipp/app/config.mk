product:=uipp_app
$(product).type:=lib
$(product).depends:=render_utils drow_render gdpp_app gdpp_evt gdpp_timer gd_dr_store plugin_app_env \
                    ui_runtime plugin_app_env uipp_sprite uipp_sprite_fsm uipp_sprite_2d uipp_sprite_cfg pluginpp_app_env \
                    ui_sprite_ui

$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)
$(product).product.c.output-includes:=inc
$(product).c.export-symbols:= $(product-base)/symbols.def

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm) \
                          $(wildcard $(product-base)/ios/*.m) \
                          $(wildcard $(product-base)/ios/SKPSMTPMessage/*.m)


#mac
$(product).mac.c.sources:=$(wildcard $(product-base)/mac/*.cpp)

#android
# $(product).android.depends:=
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp) \
							  $(wildcard $(product-base)/android/native/*.cpp) \
							  $(wildcard $(product-base)/android/native/jni/*.cpp) \
							  $(wildcard $(product-base)/android/native/utils/*.cpp)
$(product).android.java-dir+=$(product-base)/android/src

#flex
$(product).flex.depends:=cpe_spack
$(product).flex.c.sources:=$(wildcard $(product-base)/flex/*.cpp) $(wildcard $(product-base)/flex/*.c)
$(product).flex.as.sources:=$(wildcard $(product-base)/flex/*.as)
$(product).flex.sys-libraries:=builtin.abc ISpecialFile.abc CModule.abc playerglobal.abc BinaryData.abc \
                               IBackingStore.abc IVFS.abc InMemoryBackingStore.abc PlayerKernel.abc
$(product).flex.as.exports:=$(product-base)/flex/exports.txt

$(eval $(call product-def,$(product),editor))

.POHEY: $(product).protocol
drow_render.export: $(product).protocol
$(product).protocol: editor.$(product)
	rsync $(call c-source-dir-to-binary-dir,$(r.uipp_app.base),editor)/inc/protocol $(DROW_RENDER_BIN) \
			--include "*.h" -r -d
