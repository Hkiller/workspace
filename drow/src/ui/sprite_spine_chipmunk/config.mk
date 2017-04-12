product:=ui_sprite_spine_chipmunk
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_sprite_chipmunk ui_sprite_spine ui_sprite_tri
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
