#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "render/utils/ui_string_table.h"
#include "render/utils/ui_string_table_builder.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "render/model/ui_object_ref.h"
#include "ui_data_src_i.h"
#include "ui_data_src_user_i.h"
#include "ui_data_src_group_item_i.h"
#include "ui_data_src_res_i.h"
#include "ui_data_src_src_i.h"
#include "ui_data_language_i.h"

ui_data_src_t
ui_data_src_create_i(ui_data_mgr_t mgr, ui_data_src_t parent, ui_data_src_type_t type, const char * data) {
    ui_data_src_t src;
    size_t data_len = strlen(data);
    ui_product_type_t product_type = ui_product_type_of(mgr, type);

    if (data_len > 0 && data[data_len - 1] == '/') data_len--;

    src = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_src));
    if (src == NULL) {
        CPE_ERROR(mgr->m_em, "src_create: %s: alloc fail!", data);
        return NULL;
    }

    src->m_mgr = mgr;
    src->m_parent = parent;
    src->m_type = type;
    src->m_product = NULL;
    src->m_id = 0;
    src->m_language = NULL;
    src->m_base_src = NULL;
    src->m_language_src = NULL;
    src->m_data = NULL;
    src->m_using_loaded = 0;
    src->m_strings_state = ui_data_src_strings_none;
    src->m_strings = NULL;
    
    TAILQ_INIT(&src->m_childs);
    TAILQ_INIT(&src->m_using_srcs);
    TAILQ_INIT(&src->m_items);
    TAILQ_INIT(&src->m_user_srcs);
    TAILQ_INIT(&src->m_users);
    TAILQ_INIT(&src->m_using_ress);

    if (parent) {
        TAILQ_INSERT_TAIL(&parent->m_childs, src, m_next_for_parent);
    }

    TAILQ_INSERT_TAIL(&product_type->srcs, src, m_next_for_type);

    if (data) {
        if (ui_data_src_set_data(src, data) != 0) {
            ui_data_src_free(src);
            return NULL;
        }
    }
    
    return src;
}

ui_data_src_t ui_data_src_create_child(ui_data_src_t parent, ui_data_src_type_t type, const char * name) {
    return ui_data_src_create_i(parent->m_mgr, parent, type, name);
}

void ui_data_src_clear_using(ui_data_src_t src) {
    while(!TAILQ_EMPTY(&src->m_using_srcs)) {
        ui_data_src_src_free(TAILQ_FIRST(&src->m_using_srcs));
    }

    while(!TAILQ_EMPTY(&src->m_using_ress)) {
        ui_data_src_res_free(TAILQ_FIRST(&src->m_using_ress));
    }

    src->m_using_loaded = 0;
}

void ui_data_src_free(ui_data_src_t src) {
    ui_data_mgr_t mgr = src->m_mgr;
    ui_product_type_t product_type = ui_product_type_of(mgr, src->m_type);

    assert(product_type);

    while(!TAILQ_EMPTY(&src->m_items)) {
        ui_data_src_group_item_free(TAILQ_FIRST(&src->m_items));
    }
    
    /*释放子资源 */
    while(!TAILQ_EMPTY(&src->m_childs)) {
        ui_data_src_t c = TAILQ_FIRST(&src->m_childs);
        ui_data_src_free(c);
        assert(c != TAILQ_FIRST(&src->m_childs));
    }

    /*如果已经加载了数据，则释放数据 */
    if (src->m_product) {
        product_type->product_free(src->m_product);
        src->m_product = NULL;
    }

    while(!TAILQ_EMPTY(&src->m_using_ress)) {
        ui_data_src_res_t src_res = TAILQ_FIRST(&src->m_using_ress);
        assert(src_res->m_user_src == src);
        ui_data_src_res_free(src_res);
    }

    while(!TAILQ_EMPTY(&src->m_using_srcs)) {
        ui_data_src_src_t src_src = TAILQ_FIRST(&src->m_using_srcs);
        assert(src_src->m_user_src == src);
        ui_data_src_src_free(src_src);
    }
    
    /*解除所有到自己的引用 */
    while(!TAILQ_EMPTY(&src->m_user_srcs)) {
        ui_data_src_src_free(TAILQ_FIRST(&src->m_user_srcs));
    }

    if (src->m_parent) {
        TAILQ_REMOVE(&src->m_parent->m_childs, src, m_next_for_parent);
    }

    if (src->m_id) {
        cpe_hash_table_remove_by_ins(&mgr->m_srcs_by_id, src);
        src->m_id = 0;
    }

    TAILQ_REMOVE(&product_type->srcs, src, m_next_for_type);

    if (src->m_language_src) {
        ui_data_language_disconnect_src(NULL, src);
        assert(src->m_language_src == NULL);
    }

    if (src->m_base_src) {
        ui_data_language_disconnect_src(src, NULL);
        assert(src->m_base_src == NULL);
    }

    if (src->m_language) {
        TAILQ_REMOVE(&src->m_language->m_srcs, src, m_next_for_language);
        src->m_language = NULL;
    }

    if (src->m_data) {
        cpe_hash_table_remove_by_ins(&mgr->m_srcs_by_name, src);
        mem_free(mgr->m_alloc, (void*)src->m_data);
        src->m_data = NULL;
    }

    ui_data_src_strings_clear(src);
    
    assert(TAILQ_EMPTY(&src->m_users));

    mem_free(mgr->m_alloc, src);
}

uint32_t ui_data_src_id(ui_data_src_t src) {
    return src->m_id;
}

void * ui_data_src_product(ui_data_src_t src) {
    return src->m_product;
}

void ui_data_src_set_product(ui_data_src_t src, void * product) {
    assert(src->m_product == NULL);
    src->m_product = product;
}

int ui_data_src_set_id(ui_data_src_t src, uint32_t id) {
    ui_data_mgr_t mgr = src->m_mgr;
    uint32_t old_id = src->m_id;

    if (old_id) {
        cpe_hash_table_remove_by_ins(&mgr->m_srcs_by_id, src);
    }

    src->m_id = id;

    if (src->m_id) {
        cpe_hash_entry_init(&src->m_hh_for_id);
        if (cpe_hash_table_insert_unique(&mgr->m_srcs_by_id, src) != 0) {
            CPE_ERROR(mgr->m_em, "set src id: id %d duplicate", src->m_id);
            src->m_id = old_id;
            if (src->m_id) {
                cpe_hash_entry_init(&src->m_hh_for_id);
                cpe_hash_table_insert_unique(&mgr->m_srcs_by_id, src);
            }
            return -1;
        }
    }

    return 0;
}

ui_data_mgr_t ui_data_src_mgr(ui_data_src_t src) {
    return src->m_mgr;
}

ui_data_src_t ui_data_src_find_by_id(ui_data_mgr_t mgr, uint32_t id) {
    struct ui_data_src key;
    ui_data_src_t r;
    
    key.m_id = id;

    r = cpe_hash_table_find(&mgr->m_srcs_by_id, &key);

    return r
        ? (r->m_language_src ? r->m_language_src : r)
        : NULL;
}

ui_data_src_t ui_data_src_find_by_path(ui_data_mgr_t mgr, const char * path, ui_data_src_type_t type) {
    return ui_data_src_child_find_by_path(mgr->m_src_root, path, type);
}

ui_data_src_t ui_data_src_find_by_url(ui_data_mgr_t mgr, UI_OBJECT_URL const * url) {
    UI_OBJECT_SRC_REF const * src_ref;
    ui_data_src_type_t src_type;
    ui_data_src_t src;

    switch(url->type) {
    case UI_OBJECT_TYPE_IMG_BLOCK:
        src_ref = &url->data.img_block.src;
        src_type = ui_data_src_type_module;
        break;
    case UI_OBJECT_TYPE_FRAME:
        src_ref = &url->data.frame.src;
        src_type = ui_data_src_type_sprite;
        break;
    case UI_OBJECT_TYPE_ACTOR:
        src_ref = &url->data.actor.src;
        src_type = ui_data_src_type_action;
        break;
    case UI_OBJECT_TYPE_PARTICLE:
        src_ref = &url->data.particle.src;
        src_type = ui_data_src_type_particle;
        break;
    case UI_OBJECT_TYPE_SKELETON:
        src_ref = &url->data.skeleton.src;
        src_type = ui_data_src_type_spine_skeleton;
        break;
    case UI_OBJECT_TYPE_LAYOUT:
        src_ref = &url->data.layout.src;
        src_type = ui_data_src_type_layout;
        break;
    case UI_OBJECT_TYPE_MOVING:
        src_ref = &url->data.moving.src;
        src_type = ui_data_src_type_moving_plan;
        break;
    case UI_OBJECT_TYPE_CHIPMUNK:
        src_ref = &url->data.chipmunk.src;
        src_type = ui_data_src_type_chipmunk_scene;
        break;
    case UI_OBJECT_TYPE_TILEDMAP:
        src_ref = &url->data.tiledmap.src;
        src_type = ui_data_src_type_tiledmap_scene;
        break;
    case UI_OBJECT_TYPE_UI_TEMPLATE:
        src_ref = &url->data.ui_template.src;
        src_type = ui_data_src_type_layout;
        break;
    case UI_OBJECT_TYPE_EMITTER:
        src_ref = &url->data.emitter.src;
        src_type = ui_data_src_type_barrage;
        break;
    case UI_OBJECT_TYPE_SPINE_STATE_DEF:
        src_ref = &url->data.spine_state.src;
        src_type = ui_data_src_type_spine_state_def;
        break;
    case UI_OBJECT_TYPE_SCROLLMAP:
        src_ref = &url->data.scrollmap.src;
        src_type = ui_data_src_type_scrollmap_scene;
        break;
    case UI_OBJECT_TYPE_SWF:
        src_ref = &url->data.swf.src;
        src_type = ui_data_src_type_swf;
        break;
    default:
        CPE_ERROR(mgr->m_em, "ui_data_src_find_by_url: res type %d unknown!", url->type);
        return NULL;
    }

    assert(src_ref);

    switch(src_ref->type) {
	case UI_OBJECT_SRC_REF_TYPE_BY_ID:
		src = ui_data_src_find_by_id(mgr, src_ref->data.by_id.src_id);
		if (src == NULL) {
			CPE_ERROR(mgr->m_em, "ui_data_src_find_by_url: src %d not exist!", src_ref->data.by_id.src_id);
			return NULL;
		}
        break;
    case UI_OBJECT_SRC_REF_TYPE_BY_PATH:
        src = ui_data_src_find_by_path(mgr, (const char *)src_ref->data.by_path.path, src_type);
        if (src == NULL) {
            CPE_ERROR(
                mgr->m_em, "ui_data_src_find_by_url: src %s type %s not exist!",
                (const char *)src_ref->data.by_path.path, ui_data_src_type_name(src_type));
            return NULL;
        }
        break;
    default:
        CPE_ERROR(mgr->m_em, "ui_data_src_find_by_url: src ref type %d unknown!", src_ref->type);
        return NULL;
    }

    return src;
}

ui_data_src_t ui_data_src_find_by_res(ui_data_mgr_t mgr, const char * res) {
    UI_OBJECT_URL buf;
    UI_OBJECT_URL * url;

    if (res[0] == '[') {
        const char * p = strchr(res + 1, ']');
        if (p == NULL) {
            CPE_ERROR(mgr->m_em, "ui_data_src_find_by_res: res %s format error!", res);
            return NULL;
        }

        res = p + 1;
    }

    url = ui_object_ref_parse(res, &buf, mgr->m_em);
    if (url == NULL) {
        CPE_ERROR(mgr->m_em, "ui_data_src_find_by_res: res %s format error!", res);
        return NULL;
    }

    return ui_data_src_find_by_url(mgr, url);
}

ui_data_src_type_t ui_data_src_type(ui_data_src_t src) {
    return src->m_type;
}

const char * ui_data_src_data(ui_data_src_t src) {
    return src->m_data;
}

int ui_data_src_set_data(ui_data_src_t src, const char * data) {
    ui_data_mgr_t mgr = src->m_mgr;
    char * new_data;
    ui_data_language_t new_language;
    
    if (data == NULL) {
        new_language = NULL;
        new_data = NULL;
    }
    else {
        const char * sep = strrchr(data, '_');
        new_language = sep ? ui_data_language_find(mgr, sep + 1) : NULL;
        if (new_language) {
            new_data = cpe_str_mem_dup_range(mgr->m_alloc, data, sep);
        }
        else {
            new_data = cpe_str_mem_dup(mgr->m_alloc, data);
        }
        
        if (new_data == NULL) {
            CPE_ERROR(mgr->m_em, "set src data: dup str fail!");
            return -1;
        }
    }
    
    if (src->m_language) {
        TAILQ_REMOVE(&src->m_language->m_srcs, src, m_next_for_language);
        ui_data_language_disconnect_src(src, NULL);
    }
    else {
        ui_data_language_disconnect_src(NULL, src);
    }
    
    if (src->m_data) {
        cpe_hash_table_remove_by_ins(&mgr->m_srcs_by_name, src);
        mem_free(mgr->m_alloc, src->m_data);
    }

    src->m_data = new_data;
    src->m_language = new_language;

    if (src->m_language) {
        TAILQ_INSERT_TAIL(&src->m_language->m_srcs, src, m_next_for_language);
        if (src->m_language == mgr->m_active_language) {
            ui_data_language_connect_src(src->m_language, src, NULL);
        }
    }
    else {
        if (mgr->m_active_language) {
            ui_data_language_connect_src(mgr->m_active_language, NULL, src);
        }
    }
    
    if (src->m_data) {
        cpe_hash_entry_init(&src->m_hh_for_name);
        cpe_hash_table_insert(&mgr->m_srcs_by_name, src);
    }

    return 0;
}

const char * ui_data_src_data_with_language(ui_data_src_t src, char * buf, size_t buf_size) {
    if (src->m_language) {
        snprintf(buf, buf_size, "%s_%s", src->m_data, src->m_language->m_name);
        return buf;
    }
    else {
        return src->m_data;
    }
}

ui_data_language_t ui_data_src_language(ui_data_src_t src) {
    return src->m_language;
}

const char * ui_data_src_language_name(ui_data_src_t src) {
    return src->m_language ? src->m_language->m_name : "";
}

ui_data_src_t ui_data_src_language_src(ui_data_src_t src) {
    return src->m_language_src;
}

ui_data_src_t ui_data_src_base_src(ui_data_src_t src) {
    if (src->m_language == NULL) return NULL;
    if (src->m_base_src)  return src->m_base_src;
    if (src->m_parent == NULL) return NULL;
    return ui_data_src_child_find_with_language(src->m_parent, src->m_data, src->m_type, NULL);
}

ui_data_src_t ui_data_src_parent(ui_data_src_t src) {
    return src->m_parent;
}

ui_data_src_t
ui_data_src_child_find_with_language(
    ui_data_src_t src, const char * name, ui_data_src_type_t type, ui_data_language_t language)
{
    struct ui_data_src key;
    ui_data_src_t r;
    
    key.m_data = (char*)name;

    for(r = cpe_hash_table_find(&src->m_mgr->m_srcs_by_name, &key);
        r;
        r = cpe_hash_table_find_next(&src->m_mgr->m_srcs_by_name, r))
    {
        if (r->m_parent != src) continue;

        if ((type == ui_data_src_type_all || type == r->m_type) /*类型符合 */
            && r->m_language == language) /*语言匹配 */
        {
            break;
        }
    }

    return r;
}    

ui_data_src_t ui_data_src_child_find(ui_data_src_t src, const char * name, ui_data_src_type_t type) {
    ui_data_mgr_t mgr = src->m_mgr;
    struct ui_data_src key;
    ui_data_src_t r;
    
    key.m_data = (char*)name;

    for(r = cpe_hash_table_find(&src->m_mgr->m_srcs_by_name, &key);
        r;
        r = cpe_hash_table_find_next(&src->m_mgr->m_srcs_by_name, r))
    {
        if (r->m_parent != src) continue;
        
        if ((type == ui_data_src_type_all || type == r->m_type) /*类型符合 */
            && (r->m_language == NULL || r->m_language == mgr->m_active_language) /*默认或者语言匹配 */
            )
        {
            break;
        }
    }

    return r
        ? ( r->m_language_src ? r->m_language_src : r )
        : NULL;
}

struct ui_data_src_find_node {
    ui_data_src_t m_src;
    const char * m_path;
};

ui_data_src_t ui_data_src_child_find_by_path(ui_data_src_t input_src, const char * input_path, ui_data_src_type_t type) {
    ui_data_mgr_t mgr = input_src->m_mgr;
    struct ui_data_src_find_node search_stack[20];
    size_t search_stack_size;
    ui_data_src_t found_src = NULL;

    search_stack_size = 0;

    if (input_src->m_base_src) {
        search_stack[search_stack_size].m_src = input_src->m_base_src;
        search_stack[search_stack_size].m_path = input_path;
        search_stack_size++;
    }
    
    search_stack[search_stack_size].m_src = input_src;
    search_stack[search_stack_size].m_path = input_path;
    search_stack_size++;

    while(search_stack_size > 0) {
        ui_data_src_t cur_src;
        const char * cur_path;
        const char * sep;
        char name_buf[64];
        ui_data_language_t child_language;
        char * language_sep;
        uint8_t is_last = 1;
        ui_data_src_t child_src;
        
        search_stack_size--;
        cur_src = search_stack[search_stack_size].m_src;
        cur_path = search_stack[search_stack_size].m_path;

        if ((sep = strchr(cur_path, '/'))) {
            if (sep == cur_path) {
                CPE_ERROR(mgr->m_em, "src_child_find_by_path: name empty!");
                return NULL;
            }

            if (cpe_str_dup_range(name_buf, sizeof(name_buf), cur_path, sep) == NULL) {
                CPE_ERROR(mgr->m_em, "src_child_find_by_path: name overflow!");
                return NULL;
            }

            is_last = 0;
        }
        else if (cur_path[0] == 0) {
            found_src = cur_src;
            break;
        }
        else {
            if (cpe_str_dup(name_buf, sizeof(name_buf), cur_path) == NULL) {
                CPE_ERROR(mgr->m_em, "src_child_find_by_path: name overflow!");
                return NULL;
            }
            
            is_last = 1;
        }

        child_language = NULL;
        language_sep = strrchr(name_buf, '_');
        if (language_sep) {
            child_language = ui_data_language_find(mgr, language_sep + 1);
            if (child_language) *language_sep = 0;
        }
            
        child_src = child_language
            ? ui_data_src_child_find_with_language(cur_src, name_buf, is_last ? type : ui_data_src_type_dir, child_language)
            : ui_data_src_child_find(cur_src, name_buf, is_last ? type : ui_data_src_type_dir);
        if (child_src == NULL) continue;

        if (!is_last) {
            if (child_src->m_base_src) {
                if (search_stack_size + 1u >= CPE_ARRAY_SIZE(search_stack)) {
                    CPE_ERROR(mgr->m_em, "src_child_find_by_path: search static overflow!");
                    return NULL;
                }
                search_stack[search_stack_size].m_src = child_src->m_base_src;
                search_stack[search_stack_size].m_path = sep + 1;
                search_stack_size++;
            }

            if (search_stack_size + 1u >= CPE_ARRAY_SIZE(search_stack)) {
                CPE_ERROR(mgr->m_em, "src_child_find_by_path: search static overflow!");
                return NULL;
            }
            search_stack[search_stack_size].m_src = child_src;
            search_stack[search_stack_size].m_path = sep + 1;
            search_stack_size++;
        }
        else {
            found_src = child_src;
            break;
        }
    }

    if (found_src == NULL || found_src->m_type != type) return NULL;
    
    return found_src->m_language_src ? found_src->m_language_src : found_src;
}

static ui_data_src_t ui_data_src_childs_next(struct ui_data_src_it * it) {
    ui_data_src_t * data = (ui_data_src_t *)(it->m_data);
    ui_data_src_t r;

    if (*data == NULL) return NULL;

    r = *data;

    assert(r->m_parent);

    *data = TAILQ_NEXT(r, m_next_for_parent);

    return r;
}

void ui_data_src_childs(ui_data_src_it_t it, ui_data_src_t src) {
    *(ui_data_src_t *)(it->m_data) = TAILQ_FIRST(&src->m_childs);
    it->next = ui_data_src_childs_next;
}

struct ui_data_src_all_childs_it_data {
    ui_data_src_t m_root;
    ui_data_src_t m_cur;
};

static ui_data_src_t ui_data_src_all_childs_next(struct ui_data_src_it * it) {
    struct ui_data_src_all_childs_it_data * data = (struct ui_data_src_all_childs_it_data *)(it->m_data);
    ui_data_src_t r;
    ui_data_src_t n;

    if (data->m_cur == NULL) return NULL;

    r = data->m_cur;

    if (r == data->m_root) {
        n = NULL;
    }
    else {
        n = TAILQ_NEXT(r, m_next_for_parent);
        if (n) {
            while(!TAILQ_EMPTY(&n->m_childs)) {
                n = TAILQ_FIRST(&n->m_childs);
            }
        }
        else {
            n = r->m_parent;
        }
    }

    data->m_cur = n;

    return r;
}

void ui_data_src_all_childs(ui_data_src_it_t it, ui_data_src_t src) {
    struct ui_data_src_all_childs_it_data * data = (struct ui_data_src_all_childs_it_data *)(it->m_data);
    ui_data_src_t r = src;

    while(!TAILQ_EMPTY(&r->m_childs)) {
        r = TAILQ_FIRST(&r->m_childs);
    }

    data->m_root = src;
    data->m_cur = r;
    it->next = ui_data_src_all_childs_next;
}

ui_data_src_load_state_t ui_data_src_load_state(ui_data_src_t src) {
    return src->m_product == NULL
        ? ui_data_src_state_notload
        : ui_data_src_state_loaded;
}

int ui_data_src_init(ui_data_src_t src) {
    void * p;
    struct ui_product_type * product_type;

    ui_data_src_unload(src);

    product_type = ui_product_type_of(src->m_mgr, src->m_type);
    if (product_type->product_create == NULL) {
        CPE_ERROR(src->m_mgr->m_em, "%s no product type", ui_data_src_type_name(src->m_type));
        return -1;
    }

    p = product_type->product_create(product_type->product_create_ctx, src);
    if (p == NULL) return -1;

    src->m_product = p;

    return 0;
}

int ui_data_src_load(ui_data_src_t src, error_monitor_t em) {
    ui_data_mgr_t mgr = src->m_mgr;
    struct ui_product_type * product_type = ui_product_type_of(mgr, src->m_type);
    int r;

    assert(src);
    assert(product_type);
    
    if (product_type->product_load == NULL) {
        CPE_ERROR(em, "%s not support load", ui_data_src_type_name(src->m_type));
        assert(0);
        return -1;
    }

    if (src->m_product) {
        CPE_ERROR(em, "%s is already loaded", ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return -1;
    }

    if ((r = product_type->product_load(product_type->product_load_ctx, mgr, src, em))) return r;

    if (src->m_product == NULL) {
        CPE_ERROR(em, "%s load complete, no product!", ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return -1;
    }

    return 0;
}

int ui_data_src_check_load_with_usings(ui_data_src_t src, error_monitor_t em) {
    ui_data_mgr_t mgr = src->m_mgr;
    ui_data_src_src_t src_src;
    ui_data_src_src_t processing_src_ref[128];
    uint8_t processing_pos = 0;
    
    if (ui_data_src_load_state(src) != ui_data_src_state_loaded) {
        if (ui_data_src_load(src, em) != 0) {
            CPE_ERROR(
                em, "%s(%d) check load: load fail!",
                ui_data_src_path_dump(&mgr->m_dump_buffer, src), src->m_id);
            return -1;
        }
    }

    ui_data_src_update_using(src);
    
    TAILQ_FOREACH(src_src, &src->m_using_srcs, m_next_for_user) {
        if (processing_pos >= CPE_ARRAY_SIZE(processing_src_ref)) {
            CPE_ERROR(
                em, "%s(%d) check load: processing count overflow!",
                ui_data_src_path_dump(&mgr->m_dump_buffer, src), src->m_id);
            return -1;
        }

        processing_src_ref[processing_pos++] = src_src;
    }

    while(processing_pos > 0) {
        ui_data_src_t using_src;

        src_src = processing_src_ref[--processing_pos];
        using_src = src_src->m_using_src;

        assert(using_src);

        if (ui_data_src_load_state(using_src) != ui_data_src_state_loaded) {
            if (ui_data_src_load(using_src, em) != 0) {
                CPE_ERROR(
                    em, "%s(%d) check load: process %d: load fail!",
                    ui_data_src_path_dump(&mgr->m_dump_buffer, src), src->m_id, using_src->m_id);
                return -1;
            }
        }

        ui_data_src_update_using(using_src);
        TAILQ_FOREACH(src_src, &using_src->m_using_srcs, m_next_for_user) {
            if (processing_pos >= CPE_ARRAY_SIZE(processing_src_ref)) {
                CPE_ERROR(
                    em, "%s(%d) check load: process %d: processing count overflow!",
                    ui_data_src_path_dump(&mgr->m_dump_buffer, src), src->m_id, using_src->m_id);
                return -1;
            }
        
            processing_src_ref[processing_pos++] = src_src;
        }
    }

    return 0;
}

void ui_data_src_unload(ui_data_src_t src) {
    if (src->m_product) {
        ui_product_type_of(src->m_mgr, src->m_type)->product_free(src->m_product);
        src->m_product = NULL;
    }

    switch(src->m_strings_state) {
    case ui_data_src_strings_r:
        assert(src->m_strings);
        ui_string_table_free(src->m_strings);
        src->m_strings = NULL;
        src->m_strings_state = ui_data_src_strings_none;
        break;
    case ui_data_src_strings_w:
        assert(src->m_strings);
        ui_string_table_builder_free(src->m_strings);
        src->m_strings = NULL;
        src->m_strings_state = ui_data_src_strings_none;
        break;
    case ui_data_src_strings_none:
        break;
    }
    
    ui_data_src_clear_using(src);
    
    while(!TAILQ_EMPTY(&src->m_users)) {
        ui_data_src_user_t src_user = TAILQ_FIRST(&src->m_users);
        src_user->m_fun(src_user->m_ctx, src);
        assert(src_user != TAILQ_FIRST(&src->m_users));
    }
}

int ui_data_src_save(ui_data_src_t src, const char * root, error_monitor_t em) {
    ui_data_mgr_t mgr = src->m_mgr;
    struct ui_product_type * product_type = ui_product_type_of(mgr, src->m_type);
    int r;

    if (src->m_strings_state ==  ui_data_src_strings_w) {
        if (ui_data_src_strings_build_complete(src) != 0) {
            CPE_ERROR(em, "%s strings build fail", ui_data_src_type_name(src->m_type));
            return -1;
        }
    }
    
    if (product_type->product_save == NULL) {
        CPE_ERROR(em, "%s not support save", ui_data_src_type_name(src->m_type));
        return -1;
    }

    if (src->m_product == NULL) {
        CPE_ERROR(em, "%s not loaded", ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return -1;
    }

    if (root == NULL) root = mgr->m_src_root->m_data;

    if ((r = product_type->product_save(product_type->product_save_ctx, mgr, src, root, em))) return r;

    return 0;
}

int ui_data_src_remove(ui_data_src_t src, const char * root, error_monitor_t em) {
    ui_data_mgr_t mgr = src->m_mgr;
    struct ui_product_type * product_type = ui_product_type_of(mgr, src->m_type);
    int r;

    if (product_type->product_remove == NULL) {
        CPE_ERROR(em, "%s not support remove", ui_data_src_type_name(src->m_type));
        return -1;
    }

    if (root == NULL) root = mgr->m_src_root->m_data;

    if ((r = product_type->product_remove(product_type->product_save_ctx, mgr, src, root, em))) return r;

    return 0;
}

int ui_data_src_update_using(ui_data_src_t src) {
    ui_data_mgr_t mgr = src->m_mgr;
    ui_product_type_t product_type = ui_product_type_of(mgr, ui_data_src_type(src));
    ui_data_src_src_t src_using_src;
    int rv = 0;
    
    if (src->m_using_loaded) return 0;

    if (src->m_product == NULL) {
        CPE_ERROR(mgr->m_em, "%s: update using: product not loaded", ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return -1;
    }
    
    ui_data_src_clear_using(src);

    src->m_using_loaded = 1;
    if (product_type->product_update_usings) {
        void * product = ui_data_src_product(src);
        if (product) {
            rv = product_type->product_update_usings(src);
        }
    }

    TAILQ_FOREACH(src_using_src, &src->m_using_srcs, m_next_for_user) {
        if (src_using_src->m_using_src->m_using_loaded) continue;
        if (ui_data_src_update_using(src_using_src->m_using_src) != 0) rv = -1;
    }
    
    if (rv != 0) {
        ui_data_src_clear_using(src);
        return rv;
    }
    
    return 0;
}

static ui_data_src_t ui_data_src_using_src_next(ui_data_src_it_t it) {
    ui_data_src_src_t * data = (ui_data_src_src_t *)(it->m_data);
    ui_data_src_src_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_user);

    return r->m_using_src;
}

void ui_data_src_using_srcs(ui_data_src_t src, ui_data_src_it_t it) {
    *(ui_data_src_src_t *)(it->m_data) = TAILQ_FIRST(&src->m_using_srcs);
    it->next = ui_data_src_using_src_next;
}

static ui_data_src_t ui_data_src_user_src_next(ui_data_src_it_t it) {
    ui_data_src_src_t * data = (ui_data_src_src_t *)(it->m_data);
    ui_data_src_src_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_using);

    return r->m_user_src;
}

void ui_data_src_user_srcs(ui_data_src_t src, ui_data_src_it_t it) {
    *(ui_data_src_src_t *)(it->m_data) = TAILQ_FIRST(&src->m_user_srcs);
    it->next = ui_data_src_user_src_next;
}

static ui_cache_res_t ui_data_src_using_res_next(ui_cache_res_it_t it) {
    ui_data_src_res_t * data = (ui_data_src_res_t *)(it->m_data);
    ui_data_src_res_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_src);

    return r->m_using_res;
}

void ui_data_src_using_ress(ui_data_src_t src, ui_cache_res_it_t it) {
    *(ui_data_src_res_t *)(it->m_data) = TAILQ_FIRST(&src->m_using_ress);
    it->next = ui_data_src_using_res_next;
}

uint32_t ui_data_src_hash(ui_data_src_t src) {
    return cpe_hash_str(src->m_data, strlen(src->m_data));
}

int ui_data_src_eq(ui_data_src_t l, ui_data_src_t r) {
    return strcmp(l->m_data, r->m_data) == 0;
    /*
    while(l && r) {
        int cmp = strcmp(l->m_data, r->m_data);
        if (cmp != 0) return cmp;

        l = l->m_parent;
        r = r->m_parent;
    };

    return l ? -1 : (r ? 1 : 0);*/
}

uint32_t ui_data_src_id_hash(const ui_data_src_t src) {
    return src->m_id;
}

ui_data_src_strings_state_t ui_data_src_strings_state(ui_data_src_t src) {
    return src->m_strings_state;
}

ui_string_table_t ui_data_src_strings(ui_data_src_t src) {
    return src->m_strings_state == ui_data_src_strings_r ? src->m_strings : NULL;
}

const char * ui_data_src_msg(ui_data_src_t src, uint32_t msg_id) {
    if (msg_id == 0) return "";

    switch(src->m_strings_state) {
    case ui_data_src_strings_r:
        return ui_string_table_message(src->m_strings, msg_id);
    case ui_data_src_strings_w:
        return ui_string_table_builder_msg_get(src->m_strings, msg_id);
    case ui_data_src_strings_none:
    default:
        return "";
    }
}

int ui_data_src_strings_set(ui_data_src_t src, void const * data, size_t data_size) {
    switch(src->m_strings_state) {
    case ui_data_src_strings_r:
        assert(src->m_strings);
        break;
    case ui_data_src_strings_w:
        ui_string_table_builder_free(src->m_strings);
        src->m_strings = ui_string_table_create(src->m_mgr->m_alloc, src->m_mgr->m_em);
        break;
    case ui_data_src_strings_none:
    default:
        assert(src->m_strings == NULL);
        src->m_strings = ui_string_table_create(src->m_mgr->m_alloc, src->m_mgr->m_em);
        break;
    }

    if (src->m_strings == NULL) {
        CPE_ERROR(src->m_mgr->m_em, "ui_data_src_strings_set: alloc strings fail!");
        src->m_strings_state = ui_data_src_strings_none;
        return -1;
    }

    if (ui_string_table_load(src->m_strings, data, data_size) != 0) {
        CPE_ERROR(src->m_mgr->m_em, "ui_data_src_strings_set: load strings fail!");
        src->m_strings_state = ui_data_src_strings_none;
        ui_string_table_free(src->m_strings);
        src->m_strings = NULL;
        return -1;
    }
    
    src->m_strings_state = ui_data_src_strings_r;
    return 0;
}

void ui_data_src_strings_clear(ui_data_src_t src) {
    switch(src->m_strings_state) {
    case ui_data_src_strings_r:
        assert(src->m_strings);
        ui_string_table_free(src->m_strings);
        src->m_strings = NULL;
        src->m_strings_state = ui_data_src_strings_none;
        break;
    case ui_data_src_strings_w:
        assert(src->m_strings);
        ui_string_table_builder_free(src->m_strings);
        src->m_strings = NULL;
        src->m_strings_state = ui_data_src_strings_none;
        break;
    case ui_data_src_strings_none:
        break;
    }
}

ui_string_table_builder_t ui_data_src_strings_buildbuilder(ui_data_src_t src) {
    return  src->m_strings_state == ui_data_src_strings_w ? src->m_strings : NULL;
}

ui_string_table_builder_t ui_data_src_strings_build_begin(ui_data_src_t src) {
    switch(src->m_strings_state) {
    case ui_data_src_strings_w:
        return src->m_strings;
    case ui_data_src_strings_r:
        CPE_ERROR(src->m_mgr->m_em, "ui_data_src_strings_build_begin: strings builder begin, can`t begin from read!");
        return NULL;
    case ui_data_src_strings_none:
        assert(src->m_strings == NULL);
        src->m_strings = ui_string_table_builder_create(src->m_mgr->m_alloc, src->m_mgr->m_em);
        if (src->m_strings == NULL) {
            CPE_ERROR(src->m_mgr->m_em, "ui_data_src_strings_build_begin: alloc strings builder fail!");
            return NULL;
        }

        src->m_strings_state = ui_data_src_strings_w;
        return src->m_strings;
    default:
        CPE_ERROR(src->m_mgr->m_em, "ui_data_src_strings_build_begin: strings builder begin, unknown state!");
        return NULL;
    }
}

int ui_data_src_strings_build_complete(ui_data_src_t src) {
    ui_string_table_t strings;
    
    if (src->m_strings_state != ui_data_src_strings_w) {
        CPE_ERROR(src->m_mgr->m_em, "ui_data_src_strings_build_complete: no string builder!");
        return -1;
    }

    strings = ui_string_table_create(src->m_mgr->m_alloc, src->m_mgr->m_em);
    if (strings == NULL) {
        CPE_ERROR(src->m_mgr->m_em, "ui_data_src_strings_build_complete: create strings fail!");
        return -1;
    }

    if (ui_string_table_builder_build(src->m_strings, strings) != 0) {
        CPE_ERROR(src->m_mgr->m_em, "ui_data_src_strings_build_complete: build strings fail!");
        ui_string_table_free(strings);
        return -1;
    }

    ui_string_table_builder_free(src->m_strings);
    src->m_strings = strings;
    src->m_strings_state = ui_data_src_strings_r;
    
    return 0;
}

uint32_t ui_data_src_msg_alloc(ui_data_src_t src, const char * str) {
    ui_string_table_builder_t builder = ui_data_src_strings_build_begin(src);
    if (builder == NULL) return 0;
    return ui_string_table_builder_msg_alloc(builder, str);
}

void ui_data_src_msg_remove(ui_data_src_t src, uint32_t msg_id) {
    if (msg_id) {
        ui_string_table_builder_t builder = ui_data_src_strings_build_begin(src);
        ui_string_table_builder_msg_free(builder, msg_id);
    }
}

uint32_t ui_data_src_msg_update(ui_data_src_t src, uint32_t id, const char * msg) {
    ui_string_table_builder_t builder = ui_data_src_strings_build_begin(src);
    if (id) ui_string_table_builder_msg_free(builder, id);
    return ui_string_table_builder_msg_alloc(builder, msg);
}

int ui_data_src_id_eq(const ui_data_src_t l, const ui_data_src_t r) {
    return l->m_id == r->m_id;
}

void ui_data_src_data_print(write_stream_t s, ui_data_src_t src) {
    stream_printf(s, "%s", src->m_data);
    if (src->m_language) stream_printf(s, "_%s", src->m_language->m_name);
}

void ui_data_src_path_print_to(write_stream_t s, ui_data_src_t src, ui_data_src_t to) {
    if (src == to) return;

    if (src->m_parent != to) {
        if (src->m_parent->m_parent == NULL && src->m_parent->m_data[0] == 0) {
            stream_printf(s, "%s", src->m_data);
        }
        else {
            ui_data_src_path_print_to(s, src->m_parent, to);
            stream_printf(s, "/%s", src->m_data);
        }
    }
    else {
        stream_printf(s, "%s", src->m_data);
    }

    if (src->m_language) stream_printf(s, "_%s", src->m_language->m_name);
}

void ui_data_src_path_print(write_stream_t s, ui_data_src_t src) {
    ui_data_src_path_print_to(s, src, src->m_mgr->m_src_root);
}

const char * ui_data_src_path_dump(mem_buffer_t buffer, ui_data_src_t src) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    ui_data_src_path_print((write_stream_t)&stream, src);

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

uint8_t ui_data_src_in_group(ui_data_src_t src, ui_data_src_group_t group) {
    ui_data_src_group_item_t item;

    TAILQ_FOREACH(item, &src->m_items, m_next_for_src) {
        if (item->m_group == group) return 1;
    }

    return 0;
}

const char * ui_data_src_type_name(ui_data_src_type_t type) {
    switch(type) {
    case ui_data_src_type_dir:
        return "dir";
    case ui_data_src_type_module:
        return "module";
    case ui_data_src_type_sprite:
        return "sprite";
    case ui_data_src_type_action:
        return "action";
    case ui_data_src_type_layout:
        return "layout";
    case ui_data_src_type_spine_skeleton:
        return "skeleton";
    case ui_data_src_type_spine_state_def:
        return "spine-state";
    case ui_data_src_type_barrage:
        return "barrage";
    case ui_data_src_type_moving_plan:
        return "moving";
    case ui_data_src_type_chipmunk_scene:
        return "chipmunk";
    case ui_data_src_type_tiledmap_scene:
        return "map";
    case ui_data_src_type_particle:
        return "particle";
    case ui_data_src_type_scrollmap_scene:
        return "scrollmap";
    case ui_data_src_type_swf:
        return "swf";
    default:
        return "unknown";
    }
}

int ui_data_src_collect_ress(ui_data_src_t src, ui_cache_group_t res_group) {
    struct ui_cache_res_it using_res_it;
    ui_cache_res_t using_res;
    struct ui_data_src_it using_src_it;
    ui_data_src_t using_src;
    int rv = 0;
    
    ui_data_src_using_ress(src, &using_res_it);
    while((using_res = ui_cache_res_it_next(&using_res_it))) {
        if (ui_cache_group_add_res(res_group, using_res) != 0) rv = -1;
    }

    ui_data_src_using_srcs(src, &using_src_it);
    while((using_src = ui_cache_res_it_next(&using_src_it))) {
        if (ui_data_src_collect_ress(using_src, res_group) != 0) rv = -1;
    }

    return rv;
}
