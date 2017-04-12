# {{{ drow_render
product:=drow_render
$(product).type:=lib
$(product).depends:=gdpp_app gdpp_timer ui_utils ui_manip ui_cache ui_plugin_ui ui_plugin_basicanim

$(product).c.sources:=$(foreach d,\
                           src/RCommon \
                           src/RGUI \
                           src/RMath \
                           src/RRender \
                           , $(wildcard $(product-base)/$d/*.cpp))

$(product).product.c.includes:=drow/include 

#ios
$(product).ios.product.c.frameworks:=CoreGraphics UIKit Foundation OpenGLES QuartzCore OpenAL AVFoundation AudioToolbox GameKit \
                                     StoreKit MediaPlayer Security MessageUI SystemConfiguration

#cygwin
$(product).cygwin.c.sources:=$(wildcard $(product-base)/src/RGUI/cygwin/*.cpp)

#editor
$(product).editor.product.c.defs:=IN_EDITOR

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
# }}}
# {{{ export
.POHEY: drow_render.all drow_render.export

drow_render.export: $(UI_MODEL_TOOL) drow_render.all
	$(call with_message,export drow render) \
	$(if $(DROW_RENDER_BIN),\
        rsync $(r.drow_render.base)/include $(DROW_RENDER_BIN) \
			--include "*.h" \
			--include "*.hpp" \
			--include "*.inl" \
            --exclude ".git" \
			-r -d --delete \
        && \
        mkdir -p $(DROW_RENDER_BIN)/$(tools.env)/bin && cp $(UI_MODEL_TOOL) $(DROW_RENDER_BIN)/$(tools.env)/bin/$(notdir $(UI_MODEL_TOOL)) \
    , \
        echo "DROW_RENDER_BIN not set" \
	)
# }}}

