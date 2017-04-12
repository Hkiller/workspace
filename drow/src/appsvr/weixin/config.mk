product:=appsvr_weixin
$(product).type:=lib
$(product).depends:=appsvr_payment appsvr_account
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

#android
$(product).android.c.sources:=$(wildcard $(product-base)/android/*.cpp)
$(product).android.java-libs:=$(wildcard $(product-base)/android/libs/*.jar)
$(product).android.java-dir+=$(product-base)/android/src
$(product).android.manifest:=$(product-base)/android/AndroidManifest.xml
$(product).android.generates:=activity
$(product).android.generate.activity.target:=src/com/drowgames/flight/WXEntryActivity.java
$(product).android.generate.activity.cmd:=appsvr-weixin-gen-entity

# $(call appsvr-weixin-gen-entity,product-name,domain,target)
define appsvr-weixin-gen-entity

$3:
	echo "package $$(r.$1.package).wxapi;" > $$@ \
	&& echo "import com.drowgames.weixin.WeixinActivity;" >> $$@ \
	&& echo "public class WXEntryActivity extends WeixinActivity {" >> $$@ \
	&& echo "}" >> $$@

endef

#ios
$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm) $(wildcard $(product-base)/ios/*.m)
$(product).ios.product.c.include:=$(product-base)/ios/libs
$(product).ios.product.c.libraries+=WeChatSDK

DROW_RENDER_PRODUCTS+=$(product)
drow_render.all: $(product)
$(eval $(call product-def,$(product)))
