product:=gameswf
$(product).type:=lib
$(product).c.libraries:=
$(product).depends:=jpeg png
$(product).c.sources:=$(filter-out %/gameswf_render_handler_iwgl.cpp \
                                   %/gameswf_render_handler_d3d.cpp \
                                   %/gameswf_render_handler_sdl.cpp \
                                   %/gameswf_render_handler_ogl.cpp \
                                   %/gameswf_render_handler_ogles.cpp \
                                   %/gameswf_sound_handler_sdl.cpp \
                                   %/gameswf_sound_handler_openal.cpp \
                                   %/gameswf_test_ogl.cpp \
                                   %/gameswf_parser.cpp \
                                   %/gameswf_processor.cpp \
                                   , \
							$(wildcard $(CPDE_ROOT)/3rdTools/gameswf/GameSwfPort/GameSwf/gameswf/*.cpp)) \
					  $(wildcard $(CPDE_ROOT)/3rdTools/gameswf/GameSwfPort/GameSwf/gameswf/gameswf_as_classes/*.cpp) \
					  $(addprefix $(CPDE_ROOT)/3rdTools/gameswf/GameSwfPort/GameSwf/base/, \
                            membuf.cpp tu_loadlib.cpp zlib_adapter.cpp image.cpp jpeg.cpp utf8.cpp container.cpp tu_timer.cpp tu_random.cpp tu_file.cpp \
                            ear_clip_triangulate_float.cpp png_helper.cpp)

$(product).c.flags.cpp:=
$(product).product.c.includes:=3rdTools/gameswf/GameSwfPort/GameSwf

$(eval $(call product-def,$(product)))
