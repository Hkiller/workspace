product:=ui_model_tool
$(product).type:=progn 
$(product).depends:=argtable2 yajl pngquant curl cpe_spack cpe_plist cpe_xcalc cpe_utils_xml cpepp_cfg \
                    ui_model ui_model_proj ui_model_bin ui_model_ed ui_model_manip \
                    ui_plugin_barrage_manip ui_plugin_moving_manip ui_plugin_chipmunk_manip \
                    ui_plugin_tiledmap_manip ui_plugin_spine_manip ui_plugin_particle_manip \
                    ui_plugin_scrollmap_manip ui_plugin_package_manip ui_plugin_swf ui_plugin_mask_manip \
                    ui_sprite_manip ui_app_manip ui_plugin_pack 

$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c) \
                      $(wildcard $(product-base)/*.cpp)
$(product).c.linker:=cpp
$(product).c.flags.ld:=-rdynamic

$(product).c.export-symbols:=$(product-base)/symbols.def

$(eval $(call product-def,$(product),tools))

