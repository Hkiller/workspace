product:=pluginpp_app_env
$(product).type:=lib
$(product).depends:=plugin_app_env
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
