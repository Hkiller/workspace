product:=testenv.gd_timer
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.gd_app gd_timer 
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
