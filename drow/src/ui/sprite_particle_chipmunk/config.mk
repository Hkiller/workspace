product:=ui_sprite_particle_chipmunk
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_sprite_chipmunk ui_plugin_particle_chipmunk
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.export-symbols:= $(product-base)/symbols.def

$(eval $(call product-def,$(product)))
