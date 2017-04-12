#include <assert.h>
#include "spine/extension.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_texture.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin/spine/plugin_spine_atlas.h"
#include "plugin_spine_atlas_i.h"

static char* plugin_spine_atlas_malloc_string(const char * str) {
	size_t len = strlen(str) + 1;
	char* r = MALLOC(char, len);
	memcpy(r, str, len);
	return r;
}

static spAtlasPage * plugin_spine_atlas_load_page_from_model_one(
    plugin_spine_module_t spine_module, ui_cache_manager_t cache_mgr, spAtlas * atlas, const char * name, const char * path);

int plugin_spine_atlas_load_page_from_model(spAtlas * atlas, const char * name, ui_data_module_t module, ui_data_sprite_t sprite) {
    plugin_spine_module_t spine_module = (plugin_spine_module_t)atlas->rendererObject;
    uint16_t page_count = 0;
    uint16_t i;
    spAtlasPage * pages[128];
    spAtlasPage * * last_page;
    spAtlasRegion * origin_region_head = NULL;
    spAtlasRegion * * last_region;
    struct ui_data_img_block_it img_block_it;
    ui_data_img_block_t img_block;
    ui_cache_manager_t cache_mgr =
        spine_module->m_runtime
        ? ui_runtime_module_cache_mgr(spine_module->m_runtime)
        : ui_cache_manager_find_nc(spine_module->m_app, NULL);

    if (cache_mgr == NULL) {
        CPE_ERROR(spine_module->m_em, "load atlas pate %s: cache mgr not exist", name);
        goto LOAD_ERROR;
    }

    origin_region_head = atlas->regions;
    last_region = &atlas->regions;
    while(*last_region) last_region = &(*last_region)->next;
    
    /*构造所有block */
    ui_data_img_block_in_module(&img_block_it, module);
    while((img_block = ui_data_img_block_it_next(&img_block_it))) {
        UI_IMG_BLOCK const * img_block_data;
        spAtlasRegion * region;
        ui_data_frame_t frame;
        const char * block_name = ui_data_img_block_name(img_block);
        const char * texture_path = ui_data_img_block_using_texture_path(img_block);
        const char * page_name = file_name_base(texture_path, &spine_module->m_dump_buffer);

        img_block_data = ui_data_img_block_data(img_block);
        assert(img_block_data);
        
        frame = sprite ? ui_data_frame_find_by_name(sprite, block_name) : NULL;
        
        region = spAtlasRegion_create();
        region->next = *last_region;
        *last_region = region;

        region->page = NULL;
        for(i = 0; i < page_count; ++i) {
            if (strcmp(pages[i]->name, page_name) == 0) {
                region->page = pages[i];
                break;
            }
        }
        if (region->page == NULL) {
            pages[page_count] = plugin_spine_atlas_load_page_from_model_one(spine_module, cache_mgr, atlas, page_name, texture_path);
            if (pages[page_count] == NULL) goto LOAD_ERROR;
            region->page = pages[page_count++];
        }
        
        region->name = plugin_spine_atlas_malloc_string(block_name);
        region->rotate = 0;

        region->x = img_block_data->src_x;
        region->y = img_block_data->src_y;
        
        if (frame) {
            UI_FRAME const * frame_data;
            ui_data_frame_img_t img_ref;
            UI_IMG_REF const * img_ref_data;

            frame_data = ui_data_frame_data(frame);
            assert(frame_data);
            
            img_ref = frame ? ui_data_frame_img_get_at(frame, 0) : NULL;
            if (img_ref == NULL) {
                CPE_ERROR(spine_module->m_em, "load atlas pate %s region %s: frame no img ref", name, block_name);
                goto LOAD_ERROR;
            }
            img_ref_data = ui_data_frame_img_data(img_ref);
            assert(img_ref_data);

            if (img_ref_data->trans.local_trans.angle != 90.0f && img_ref_data->trans.local_trans.angle != 0.0f) {
                CPE_ERROR(
                    spine_module->m_em, "load atlas pate %s region ≈%s: img ref angle %f error",
                    name, block_name, img_ref_data->trans.local_trans.angle);
                goto LOAD_ERROR;
            }

            region->rotate = img_ref_data->trans.local_trans.angle == 90.0f ? 1 : 0;

            if (region->rotate) {
                region->width = img_block_data->src_h;
                region->height = img_block_data->src_w;
            }
            else {
                region->width = img_block_data->src_w;
                region->height = img_block_data->src_h;
            }
            
            region->originalWidth = frame_data->bounding.rt - frame_data->bounding.lt;
            region->originalHeight = frame_data->bounding.bm - frame_data->bounding.tp;
            region->offsetX = - frame_data->bounding.lt;
            region->offsetY = region->originalHeight + frame_data->bounding.tp;

        }
        else {
            region->width = img_block_data->src_w;
            region->height = img_block_data->src_h;
            region->originalWidth = region->width;
            region->originalHeight = region->height;
            region->offsetX = 0;
            region->offsetY = 0;
            region->rotate = 0;
        }

        /* printf("xxxxxx: %s: pos=(%d,%d), size=(%d,%d), origin-size=(%d,%d), offset=(%d,%d), rotate=%d\n",  */
        /*        region->name,  */
        /*        region->x, region->y, */
        /*        region->width, region->height, */
        /*        region->originalWidth, region->originalHeight, */
        /*        region->offsetX, region->offsetY, */
        /*        region->rotate); */

        region->u = region->x / (float)region->page->width;
        region->v = region->y / (float)region->page->height;

        if (region->rotate) {
            region->u2 = (region->x + region->height) / (float)region->page->width;
            region->v2 = (region->y + region->width) / (float)region->page->height;
        } else {
            region->u2 = (region->x + region->width) / (float)region->page->width;
            region->v2 = (region->y + region->height) / (float)region->page->height;
        }
        
        region->index = -1;
    }

    /*将页面安插进去 */
    last_page = &atlas->pages;
    while(*last_page) last_page = &(*last_page)->next;
    for(i = 0; i < page_count; ++i) {
        *last_page = pages[i];
        last_page = &(*last_page)->next;
    }

    return 0;

LOAD_ERROR:
    if (origin_region_head) atlas->regions = origin_region_head;

    for(i = 0; i < page_count; ++i) {
        spAtlasPage_dispose(pages[i]);
    }
    
    return -1;
}


spAtlas * plugin_spine_atlas_create(plugin_spine_module_t module) {
    spAtlas * atlas = (spAtlas*)NEW(spAtlas);
    atlas->rendererObject = module;
    return atlas;
}

spAtlas * plugin_spine_atlas_clone(spAtlas * from_atlas) {
    plugin_spine_module_t module = from_atlas->rendererObject;
    spAtlas * new_atlas;
    spAtlasPage * from_page;

    assert(module);

    new_atlas = plugin_spine_atlas_create(module);
    if (new_atlas == NULL) return NULL;
    
    for(from_page = from_atlas->pages; from_page; from_page = from_page->next) {
        spAtlasPage * new_page = spAtlasPage_create(new_atlas, from_page->name);
        if (new_page == NULL) {
            CPE_ERROR(module->m_em, "atlas clone: create page fail");
            goto CLONE_ERROR;
        }
    
        /* page->width = ui_cache_res_pixel_width(texture); */
        /* page->height = ui_cache_res_pixel_height(texture); */
        /* page->format = SP_ATLAS_ALPHA; */
        /* page->minFilter = SP_ATLAS_LINEAR; */
        /* page->magFilter = SP_ATLAS_LINEAR; */
        /* page->rendererObject = texture; */
        /* page->width = ui_cache_res_width(texture); */
        /* page->height = ui_cache_res_height(texture); */
        /* page->uWrap = 0; */
        /* page->vWrap = 0; */

    }

    return new_atlas;

CLONE_ERROR:
    if (new_atlas) plugin_spine_atlas_free(new_atlas);
    return NULL;
}

void plugin_spine_atlas_free(spAtlas * atlas) {
    spAtlas_dispose(atlas);
}

spAtlas *
plugin_spine_atlas_load_from_model(plugin_spine_module_t spine_module, const char * path) {
    spAtlas * atlas = NULL;
    ui_data_src_t module_src;
    ui_data_module_t module;
    ui_data_src_t base_module_src;
    ui_data_src_t sprite_src;
    ui_data_sprite_t sprite;
    const char * name = strrchr(path, '/');
    if (name == NULL) name = path;

    module_src = ui_data_src_find_by_path(spine_module->m_data_mgr, path, ui_data_src_type_module);
    if (module_src == NULL) {
        CPE_ERROR(spine_module->m_em, "load atlas from model %s: associate module not exist", path);
        goto LOAD_ERROR;
    }

    if (!ui_data_src_is_loaded(module_src)) {
        if (ui_data_src_load(module_src, spine_module->m_em) != 0) {
            CPE_ERROR(spine_module->m_em, "load atlas from model %s: associate module load fail", path);
            goto LOAD_ERROR;
        }
    }
    
    module = (ui_data_module_t)ui_data_src_product(module_src);
    assert(module);

    sprite_src = ui_data_src_find_by_path(spine_module->m_data_mgr, path, ui_data_src_type_sprite);
    if (sprite_src == NULL) {
        sprite = NULL;
    }
    else {
        if (!ui_data_src_is_loaded(sprite_src)) {
            if (ui_data_src_load(sprite_src, spine_module->m_em) != 0) {
                CPE_ERROR(spine_module->m_em, "load atlas from model %s: associate sprite load fail", path);
                goto LOAD_ERROR;
            }
        }
        
        sprite = (ui_data_sprite_t)ui_data_src_product(sprite_src);
        assert(sprite);
    }

    atlas = plugin_spine_atlas_create(spine_module);
    if (atlas == NULL) {
        CPE_ERROR(spine_module->m_em, "load atlas from model %s: create atlas fail", path);
        goto LOAD_ERROR;
    }

    if (plugin_spine_atlas_load_page_from_model(atlas, name, module, sprite) != 0) {
        CPE_ERROR(spine_module->m_em, "load atlas from model %s: load atlas from model fail", path);
        goto LOAD_ERROR;
    }

    if ((base_module_src = ui_data_src_base_src(module_src))) {
        if (!ui_data_src_is_loaded(base_module_src)) {
            if (ui_data_src_load(base_module_src, spine_module->m_em) != 0) {
                CPE_ERROR(
                    spine_module->m_em, "load atlas from model %s: associate module load fail",
                    ui_data_src_path_dump(gd_app_tmp_buffer(spine_module->m_app), base_module_src));
                goto LOAD_ERROR;
            }
        }
        
        if (plugin_spine_atlas_load_page_from_model(atlas, name, ui_data_src_product(base_module_src), sprite) != 0) {
            CPE_ERROR(spine_module->m_em, "load atlas from model %s: load atlas from model fail", path);
            goto LOAD_ERROR;
        }
    }
    
    return atlas;

LOAD_ERROR:
    if (atlas) plugin_spine_atlas_free(atlas);

    return NULL;
}

static spAtlasPage * plugin_spine_atlas_load_page_from_model_one(
    plugin_spine_module_t spine_module, ui_cache_manager_t cache_mgr, spAtlas * atlas, const char * name, const char * texture_path)
{
    ui_cache_res_t texture;
    spAtlasPage * page;

    texture = ui_cache_res_find_by_path(cache_mgr, texture_path);
    if (texture == NULL) {
        texture = ui_cache_res_create(cache_mgr, ui_cache_res_type_texture);
        if (texture == NULL) {
            CPE_ERROR(spine_module->m_em, "spine: load texture %s: create texture fail!", texture_path);
            return NULL;
        }

        if (ui_cache_res_set_path(texture, texture_path) != 0) {
            CPE_ERROR(spine_module->m_em, "spine: load texture %s: set texture path!", texture_path);
            return NULL;
        }
    }

    if (ui_cache_texture_width(texture) <= 0 || ui_cache_texture_height(texture) <= 0) {
        if (ui_cache_res_load_state(texture) == ui_cache_res_not_load
            || ui_cache_res_load_state(texture) == ui_cache_res_loading)
        {
            ui_cache_res_load_sync(texture, ui_data_src_data(ui_data_mgr_src_root(spine_module->m_data_mgr)));
        }

        if (ui_cache_res_load_state(texture) != ui_cache_res_loaded) {
            CPE_ERROR(spine_module->m_em, "spine: load texture %s: load fail, reason=%d!", texture_path, (int)ui_cache_res_load_result(texture));
            return NULL;
        }
    }

    page = spAtlasPage_create(atlas, name);
    if (page == NULL) {
        CPE_ERROR(spine_module->m_em, "load atlas pate %s: create fail", name);
        return NULL;
    }
    
    page->width = ui_cache_texture_width(texture);
    page->height = ui_cache_texture_height(texture);

    switch(ui_cache_texture_format(texture)) {
    case ui_cache_pf_pala8:
        page->format = SP_ATLAS_ALPHA;
        break;
    case ui_cache_pf_r4g4b4a4:
        page->format = SP_ATLAS_RGBA4444;
        break;
    case ui_cache_pf_r8g8b8:
        page->format = SP_ATLAS_RGB888;
        break;
    case ui_cache_pf_r8g8b8a8:
        page->format = SP_ATLAS_RGBA8888;
        break;
    default:
        CPE_ERROR(
            spine_module->m_em, "spine: load texture %s: not support format %d!",
            texture_path, ui_cache_texture_format(texture));
        spAtlasPage_dispose(page);
        return NULL;
    }

    page->minFilter = SP_ATLAS_LINEAR;
    page->magFilter = SP_ATLAS_LINEAR;
    page->rendererObject = texture;
    page->width = ui_cache_texture_width(texture);
    page->height = ui_cache_texture_height(texture);
    
    page->uWrap = 0;
    page->vWrap = 0;

    return page;
}
