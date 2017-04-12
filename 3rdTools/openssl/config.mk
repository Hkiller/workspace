product:=openssl
$(product).type:=virtual
$(product).depends:=zlib pthread
$(product).version:=openssl-1.0.1f

$(product).mac.product.c.includes+=3rdTools/openssl/include 3rdTools/openssl/include/mac
$(product).mac.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/mac
$(product).mac.product.c.libraries+=ssl crypto

$(product).ios.product.c.includes+=3rdTools/openssl/include 3rdTools/openssl/include/ios
$(product).ios.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/ios
$(product).ios.product.c.libraries+=ssl crypto

$(product).linux32.product.c.includes+=3rdTools/openssl/include 3rdTools/openssl/include/linux32
$(product).linux32.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/linux32
$(product).linux32.product.c.libraries+=ssl crypto

$(product).linux64.product.c.includes+=3rdTools/openssl/include 3rdTools/openssl/include/linux64
$(product).linux64.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/linux64
$(product).linux64.product.c.libraries+=ssl crypto

$(product).cygwin.product.c.includes+=3rdTools/openssl/include 3rdTools/openssl/include/cygwin
$(product).cygwin.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/cygwin
$(product).cygwin.product.c.libraries+=ssl crypto

$(product).emscripten.product.c.includes+=3rdTools/openssl/include 3rdTools/openssl/include/emscripten
$(product).emscripten.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/emscripten
$(product).emscripten.product.c.libraries+=ssl crypto

$(product).flex.product.c.includes+=3rdTools/openssl/include 3rdTools/openssl/include/flex
$(product).flex.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/flex
$(product).flex.product.c.libraries+=ssl crypto

$(product).android.product.c.includes:=3rdTools/openssl/android/include
$(product).android.version:=1.0.1u
$(product).android.depends:=openssl-crypto openssl-ssl

$(eval $(call product-def,$(product)))

product:=openssl-crypto
$(product).type:=virtual
$(product).android.mk:=$(product-base)/android/lib/Android-crypto.mk
$(eval $(call product-def,$(product)))

product:=openssl-ssl
$(product).type:=virtual
$(product).android.mk:=$(product-base)/android/lib/Android-ssl.mk
$(eval $(call product-def,$(product)))
