product:=ui_sprite_cfg
$(product).type:=lib
$(product).depends:=gd_dr_store ui_sprite ui_sprite_fsm
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
