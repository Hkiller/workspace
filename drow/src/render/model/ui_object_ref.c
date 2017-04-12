#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_object_ref.h"

static int ui_object_ref_parse_process_source_and_id_by_type(
    const char * res, error_monitor_t em, const char * args, UI_OBJECT_URL_DATA_SRC_ID * src_ref) {
    const char * res_id_sep = strrchr(args, '#');
    char * endp = NULL;

    if (res_id_sep == NULL) {
        CPE_ERROR(em, "parse ui url(%s): by type: img _size too small!", res);
        return -1;
    }

    src_ref->src.type = UI_OBJECT_SRC_REF_TYPE_BY_ID;

    src_ref->src.data.by_id.src_id = (uint32_t)strtoul(args, &endp, 10);
    if (endp == NULL || *endp != '#') {
        CPE_ERROR(em, "parse ui url(%s): by type: read src id fail!", res);
        return -1;
    }

    src_ref->id = (uint32_t)strtoul(res_id_sep + 1, &endp, 10);
	if (endp == NULL || *endp != 0) {
        src_ref->id = (uint32_t)-1;
        cpe_str_dup(src_ref->name, sizeof(src_ref->name), res_id_sep);
	}
    else {
        src_ref->name[0] = 0;
    }

    return 0;
}

static int ui_object_ref_parse_process_source_and_id_by_path(
    const char * res, error_monitor_t em, const char * path_end, const char * res_id, UI_OBJECT_URL_DATA_SRC_ID * src_ref)
{
    size_t path_len = path_end - res;
    char * endp = NULL;

    if (path_len + 1 > CPE_ARRAY_SIZE(src_ref->src.data.by_path.path)) {
        CPE_ERROR(em, "parse ui url(%s): by path: path len %d overflow, capacity=%d!", res, (int)path_len, (int)CPE_ARRAY_SIZE(src_ref->src.data.by_path.path));
        return -1;
    }

    src_ref->src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;
    memcpy(src_ref->src.data.by_path.path, res, path_len);
    src_ref->src.data.by_path.path[path_len] = 0;

    src_ref->id = (uint32_t)strtoul(res_id, &endp, 10);
    if (endp == NULL || (*endp != ' ' && *endp != '\t' && *endp != 0)) {
        src_ref->id = (uint32_t)-1;
        cpe_str_dup(src_ref->name, sizeof(src_ref->name), res_id);
	}
    else {
        src_ref->name[0] = 0;
    }
        
    return 0;
}

static int ui_object_ref_parse_process_skeleton(
    const char * res, error_monitor_t em, const char * args, UI_OBJECT_URL_DATA_SKELETON * skeleton)
{
    const char * res_id_sep;
    size_t path_len;

    res_id_sep = strrchr(args, '#');
    if (res_id_sep == NULL) {
        CPE_ERROR(em, "parse ui url(%s): skeleton: sep not exsit!", res);
        return -1;
    }

    path_len = res_id_sep - args;
    if (path_len + 1 > CPE_ARRAY_SIZE(skeleton->src.data.by_path.path)) {
        CPE_ERROR(em, "parse ui url(%s): skeleton: path len %d overflow, capacity=%d!", res, (int)path_len, (int)CPE_ARRAY_SIZE(skeleton->src.data.by_path.path));
        return -1;
    }

    skeleton->src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;
    memcpy(skeleton->src.data.by_path.path, args, path_len);
    skeleton->src.data.by_path.path[path_len] = 0;

    cpe_str_dup(skeleton->anim_def, sizeof(skeleton->anim_def), res_id_sep ? (res_id_sep + 1) : "");
    
    return 0;
}

static
UI_OBJECT_URL * ui_object_ref_parse_process_ty_type(
    const char * res, UI_OBJECT_URL * buf, error_monitor_t em,
    const char * type_begin, size_t type_len, const char * args)
{
    UI_OBJECT_URL * result = buf;

    if(cpe_str_cmp_part(type_begin, type_len, "img-block") == 0) {
        result->type = UI_OBJECT_TYPE_IMG_BLOCK;
        if (ui_object_ref_parse_process_source_and_id_by_type(res, em, args, &result->data.img_block) != 0) return NULL;
    }
    else if(cpe_str_cmp_part(type_begin, type_len, "frame") == 0) {
        result->type = UI_OBJECT_TYPE_FRAME;
        if (ui_object_ref_parse_process_source_and_id_by_type(res, em, args, &result->data.frame) != 0) return NULL;
    }
    else if(cpe_str_cmp_part(type_begin, type_len, "actor") == 0) {
        result->type = UI_OBJECT_TYPE_ACTOR;
        if (ui_object_ref_parse_process_source_and_id_by_type(res, em, args, &result->data.actor) != 0) return NULL;
    }
    else if(cpe_str_cmp_part(type_begin, type_len, "skeleton") == 0) {
        result->type = UI_OBJECT_TYPE_SKELETON;
        if (ui_object_ref_parse_process_skeleton(res, em, args, &result->data.skeleton) != 0) {
            return NULL;
        }
    }
    else if(cpe_str_cmp_part(type_begin, type_len, "ui-template") == 0
        || cpe_str_cmp_part(type_begin, type_len, "Template") == 0) {
        result->type = UI_OBJECT_TYPE_UI_TEMPLATE;
        result->data.ui_template.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;
        cpe_str_dup(
            result->data.ui_template.src.data.by_path.path,
            sizeof(result->data.ui_template.src.data.by_path.path), args);
    }
    else {
        result->type = UI_OBJECT_TYPE_NONE;
        cpe_str_dup_range(result->data.unknown_type.type_name, sizeof(result->data.unknown_type.type_name), type_begin, type_begin + type_len);
    }

    return result;
}

static UI_OBJECT_URL * ui_object_ref_parse_process_by_path(const char * res, UI_OBJECT_URL * buf, error_monitor_t em) {
    UI_OBJECT_URL * result = buf;
    const char * file;
    const char * sep;
    const char * postfix;
    size_t postfix_len;
    const char * res_id_sep;
	size_t path_len;
    if ((sep = strrchr(res, '/')))  {
        file = sep + 1;
    }
    else {
        file = res;
    }

    if ((sep = strrchr(file, '.'))) {
        postfix = sep + 1;
    }
    else {
        CPE_ERROR(em, "parse ui url(%s): by path: no postfix!", res);
        return NULL;
    }

    res_id_sep = strrchr(postfix, '#');
    if (res_id_sep == NULL) {
		postfix_len = strlen(postfix);
    }
	else {
		postfix_len = res_id_sep - postfix;
	}

    if (cpe_str_cmp_part(postfix, postfix_len, "act") == 0) {
        result->type = UI_OBJECT_TYPE_ACTOR;
		if (res_id_sep == NULL) {
            path_len = postfix - res - 1;
            result->data.actor.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;
            memcpy(result->data.actor.src.data.by_path.path, res, path_len);
            result->data.actor.src.data.by_path.path[path_len] = 0;
            result->data.actor.id = (uint32_t)-1;
            result->data.actor.name[0] = 0;
		}
        else {
            if (ui_object_ref_parse_process_source_and_id_by_path(
                    res, em, postfix - 1, res_id_sep + 1, &result->data.actor)
                != 0)
            {
                return NULL;
            }
        }
    }
    else if (cpe_str_cmp_part(postfix, postfix_len, "ibk") == 0) {
		result->type = UI_OBJECT_TYPE_IMG_BLOCK;
		if (res_id_sep == NULL) {
            path_len = postfix - res - 1;
            result->data.img_block.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;
            memcpy(result->data.img_block.src.data.by_path.path, res, path_len);
            result->data.img_block.src.data.by_path.path[path_len] = 0;
            result->data.img_block.id = (uint32_t)-1;
            result->data.img_block.name[0] = 0;
		}
        else {
            if (ui_object_ref_parse_process_source_and_id_by_path(
                    res, em, postfix - 1, res_id_sep + 1, &result->data.img_block)
                != 0)
            {
                return NULL;
            }
        }
    }
	else if (cpe_str_cmp_part(postfix, postfix_len, "frm") == 0) {
		result->type = UI_OBJECT_TYPE_FRAME;
		if (res_id_sep == NULL) {
            path_len = postfix - res - 1;
            result->data.frame.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;
            memcpy(result->data.frame.src.data.by_path.path, res, path_len);
            result->data.frame.src.data.by_path.path[path_len] = 0;
            result->data.frame.id = (uint32_t)-1;
            result->data.frame.name[0] = 0;
		}
        else {
            if (ui_object_ref_parse_process_source_and_id_by_path(
                    res, em, postfix - 1, res_id_sep + 1, &result->data.frame)
                != 0)
            {
                return NULL;
            }
        }
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "particle") == 0) {
		if (res_id_sep != NULL) {
			CPE_ERROR(em, "parse ui url(%s): by path: particle no id!", res);
			return NULL;
		}
		
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_PARTICLE;
		result->data.particle.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.particle.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.particle.src.data.by_path.path, res, path_len);
		result->data.particle.src.data.by_path.path[path_len] = 0;
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "spine") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_SKELETON;
		result->data.skeleton.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.skeleton.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.skeleton.src.data.by_path.path, res, path_len);
		result->data.skeleton.src.data.by_path.path[path_len] = 0;
        cpe_str_dup(result->data.skeleton.anim_def, sizeof(result->data.skeleton.anim_def), res_id_sep ? (res_id_sep + 1) : "");
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "swf") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_SWF;
		result->data.swf.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.swf.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.swf.src.data.by_path.path, res, path_len);
		result->data.swf.src.data.by_path.path[path_len] = 0;
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "mask") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_MASK;
		result->data.mask.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.mask.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.mask.src.data.by_path.path, res, path_len);
		result->data.mask.src.data.by_path.path[path_len] = 0;
        cpe_str_dup(result->data.mask.block, sizeof(result->data.mask.block), res_id_sep ? (res_id_sep + 1) : "");
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "moving") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_MOVING;
		result->data.moving.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.moving.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.moving.src.data.by_path.path, res, path_len);
		result->data.moving.src.data.by_path.path[path_len] = 0;
        cpe_str_dup(result->data.moving.node, sizeof(result->data.moving.node), res_id_sep ? (res_id_sep + 1) : "");
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "chipmunk") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_CHIPMUNK;
		result->data.chipmunk.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.chipmunk.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.chipmunk.src.data.by_path.path, res, path_len);
		result->data.chipmunk.src.data.by_path.path[path_len] = 0;
        cpe_str_dup(result->data.chipmunk.body, sizeof(result->data.chipmunk.body), res_id_sep ? (res_id_sep + 1) : "");
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "spine-state") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_SPINE_STATE_DEF;
		result->data.spine_state.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.chipmunk.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.spine_state.src.data.by_path.path, res, path_len);
		result->data.spine_state.src.data.by_path.path[path_len] = 0;
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "map") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_TILEDMAP;
		result->data.tiledmap.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.tiledmap.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.tiledmap.src.data.by_path.path, res, path_len);
		result->data.tiledmap.src.data.by_path.path[path_len] = 0;
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "lay") == 0 || cpe_str_cmp_part(postfix, postfix_len, "npTemplate") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_LAYOUT;
		result->data.layout.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;
		memcpy(result->data.layout.src.data.by_path.path, res, path_len);
		result->data.layout.src.data.by_path.path[path_len] = 0;
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "emitter") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_EMITTER;
		result->data.emitter.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.emitter.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.emitter.src.data.by_path.path, res, path_len);
		result->data.emitter.src.data.by_path.path[path_len] = 0;
	}
	else if (cpe_str_cmp_part(postfix, postfix_len, "scrollmap") == 0) {
		path_len = postfix - res - 1;
		result->type = UI_OBJECT_TYPE_SCROLLMAP;
		result->data.scrollmap.src.type = UI_OBJECT_SRC_REF_TYPE_BY_PATH;

        if ((path_len + 1) > CPE_ARRAY_SIZE(result->data.scrollmap.src.data.by_path.path)) {
            CPE_ERROR(em, "parse ui url(%s): path len %d overflow!", res, (int)path_len);
        }
        
		memcpy(result->data.scrollmap.src.data.by_path.path, res, path_len);
		result->data.scrollmap.src.data.by_path.path[path_len] = 0;
	}
	else {
        CPE_ERROR(em, "parse ui url(%s): by path: postfix %s unknown!", res, postfix);
        return NULL;
    }

    return result;
}

UI_OBJECT_URL * ui_object_ref_parse(const char * res, UI_OBJECT_URL * buf, error_monitor_t em) {
    const char * protocol_sep = strchr(res, ':');

    if (protocol_sep && cpe_str_char_range(res, protocol_sep, '.') == NULL) {
        return ui_object_ref_parse_process_ty_type(
            res, buf, em,
            res, protocol_sep - res, protocol_sep + 1);
    }
    else {
        return ui_object_ref_parse_process_by_path(res, buf, em);
    }
}
