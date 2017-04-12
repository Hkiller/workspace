#include <assert.h>
#include "cpe/pal/pal_limits.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_token.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data_entry.h"
#include "ui/sprite/ui_sprite_world_attr.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui_sprite_data_build_i.h"
#include "ui_sprite_entity_i.h"

xtoken_t ui_sprite_data_find_value(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em) {
    struct ui_sprite_data_find_ctx * ctx = input_ctx;
    xtoken_t r;

    while (attr_name[0] == '[') {
        char name_buf[64];
        const char * entity_name = cpe_str_trim_head((char*)attr_name + 1);
        const char * entity_name_end = strchr(entity_name, ']');
        size_t entity_name_len;

        if (entity_name_end == NULL) {
            CPE_ERROR(em, "ui_sprite_data_find_value: %s format error!", attr_name);
            return NULL;
        }

        entity_name_len = entity_name_end - entity_name;
        if ((entity_name_len + 1) > CPE_ARRAY_SIZE(name_buf)) {
            CPE_ERROR(em, "ui_sprite_data_find_value: entity name len %d overflow!", (int)entity_name_len);
            return NULL;
        }

        memcpy(name_buf, entity_name, entity_name_len);
        name_buf[entity_name_len] = 0;

        if (name_buf[0] == '@') {
            xtoken_t v = ui_sprite_data_find_value(input_ctx, computer, name_buf + 1, em);
            if (v == NULL) {
                CPE_ERROR(em, "ui_sprite_data_find_value: %s: calc inner arg %s fail!", attr_name, name_buf + 1);
                return NULL;
            }

            switch(xtoken_data_type(v)) {
            case xtoken_data_int: {
                int64_t int_v;
                xtoken_try_to_int64(v, &int_v);

                if (int_v < 0 || int_v > UINT32_MAX) {
                    CPE_ERROR(em, "ui_sprite_data_find_value: %s: from arg %s: entity id "FMT_INT64_T" error!", attr_name, name_buf + 1, int_v);
                    xcomputer_free_token(computer, v);
                    return NULL;
                }

                if (int_v == 0) {
                    xcomputer_free_token(computer, v);
                    return NULL;
                }

                ctx->m_entity = ui_sprite_entity_find_by_id(ctx->m_world, (uint32_t)int_v);
                if (ctx->m_entity == NULL) {
                    CPE_ERROR(em, "ui_sprite_data_find_value: %s: from arg %s: entity "FMT_INT64_T" not exist!", attr_name, name_buf + 1, int_v);
                    xcomputer_free_token(computer, v);
                    return NULL;
                }
                break;
            }
            case xtoken_data_str: {
                const char * str_v = xtoken_try_to_str(v);
                assert(str_v);

                if (str_v[0] == 0) {
                    xcomputer_free_token(computer, v);
                    return NULL;
                }

                ctx->m_entity = ui_sprite_entity_find_by_name(ctx->m_world, str_v);
                if (ctx->m_entity == NULL) {
                    CPE_ERROR(em, "ui_sprite_data_find_value: %s: from arg %s: entity %s not exist!", attr_name, name_buf + 1, str_v);
                    xcomputer_free_token(computer, v);
                    return NULL;
                }
                break;
            }
            default:
                CPE_ERROR(
                    em, "ui_sprite_data_find_value: %s: calc inner arg %s: result type %d fail!",
                    attr_name, name_buf + 1, xtoken_data_type(v));
                xcomputer_free_token(computer, v);
                return NULL;
            }

            xcomputer_free_token(computer, v);
        }
        else {
            ctx->m_entity = ui_sprite_entity_find_by_name(ctx->m_world, name_buf);
            if (ctx->m_entity == NULL) {
                CPE_ERROR(em, "ui_sprite_data_find_value: entity %s not exist!", name_buf);
                return NULL;
            }
        }

        attr_name = cpe_str_trim_head((char*)entity_name_end + 1);
    }

    if (strcmp(attr_name, "entity-id") == 0) {
        if (ctx->m_entity == NULL) {
            CPE_ERROR(em, "ui_sprite_data_find_value: %s: no associate entity!", attr_name);
            return NULL;
        }

        r = xcomputer_alloc_token(computer);
        if (r == NULL) {
            CPE_ERROR(em, "ui_sprite_data_find_value: %s: alloc token fail!", attr_name);
            return NULL;
        }

        xcomputer_set_token_int(computer, r, ui_sprite_entity_id(ctx->m_entity));

        return r;
    }
    else if (strcmp(attr_name, "entity-name") == 0) {
        if (ctx->m_entity == NULL) {
            CPE_ERROR(em, "ui_sprite_data_find_value: %s: no associate entity!", attr_name);
            return NULL;
        }

        r = xcomputer_alloc_token(computer);
        if (r == NULL) {
            CPE_ERROR(em, "ui_sprite_data_find_value: %s: alloc token fail!", attr_name);
            return NULL;
        }

        if (xcomputer_set_token_str(computer, r, ui_sprite_entity_name(ctx->m_entity)) != 0) {
            CPE_ERROR(em, "ui_sprite_data_find_value: %s: set token str!", attr_name);
            xcomputer_free_token(computer, r);
            return NULL;
        }

        return r;
    }
    else {
        struct dr_data_entry from_attr_buf;
        dr_data_entry_t from_attr = dr_data_entry_search_in_source(&from_attr_buf, ctx->m_data_source, attr_name);

        if (from_attr == NULL && ctx->m_entity) {
            from_attr = ui_sprite_entity_find_attr(&from_attr_buf, ctx->m_entity, attr_name);
        }

        if (from_attr == NULL) {
            from_attr = ui_sprite_world_find_attr(&from_attr_buf, ctx->m_world, attr_name);
        }

        if (from_attr == NULL) {
            CPE_ERROR(em, "ui_sprite_data_find_value: %s: entry not exist!", attr_name);
            return NULL;
        }

        return dr_create_token_from_entry(computer, from_attr);
    }
}

int ui_sprite_data_build(
    dr_data_entry_t to, char * arg_value,
    ui_sprite_world_t world, ui_sprite_entity_t entity, dr_data_source_t data_source)
{
    ui_sprite_repository_t repo = world->m_repo;

    if (dr_entry_type(to->m_entry) <= CPE_DR_TYPE_COMPOSITE) {
        ui_sprite_entity_t from_entity = entity;
        struct dr_data_entry from_attr_buf;
        dr_data_entry_t from_attr;

        arg_value = cpe_str_trim_head(arg_value);
        if (*arg_value != '@') {
            CPE_ERROR(
                repo->m_em, "ui_sprite_data_build: set %s(%s): can`t from not var entry!",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
            return -1;
        }
        arg_value += 1;

        if (*arg_value == '[') {
            char * context_arg = arg_value + 1;
            char * context_arg_end = (char*)cpe_str_char_not_in_pair(context_arg, ']', "{'(", ")'}");
            if (context_arg_end == NULL) {
                CPE_ERROR(
                    repo->m_em, "ui_sprite_data_build: set %s(%s): find context pair '[]' fail, arg_value=%s!",
                    dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), arg_value);
                return -1;
            }

            *context_arg_end = 0;
            arg_value = context_arg_end + 1;

            if (context_arg[0] == '@') {
                struct dr_data_entry context_attr_buf;
                dr_data_entry_t context_attr;
                const char * str_value;

                context_attr = dr_data_entry_search_in_source(&context_attr_buf, data_source, context_arg + 1);
                if (context_attr == NULL) {
                    context_attr = ui_sprite_entity_find_attr(&context_attr_buf, from_entity, context_arg + 1);
                }

                if (context_attr == NULL) {
                    CPE_ERROR(
                        repo->m_em, "ui_sprite_data_build: set %s(%s): context arg %s not exist!",
                        dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), context_arg + 1);
                    return -1;
                }

                if((str_value = dr_entry_read_string(context_attr->m_data, context_attr->m_entry)) && str_value[0]) {
                    from_entity = ui_sprite_entity_find_by_name(world, context_arg);
                    if (from_entity == NULL) {
                        CPE_ERROR(
                            repo->m_em, "ui_sprite_data_build: set %s(%s): context entity %s(%s) not exist!",
                            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), str_value, context_arg);
                        return -1;
                    }
                }
                else {
                    uint32_t entity_id;
                    if (dr_entry_try_read_uint32(&entity_id, context_attr->m_data, context_attr->m_entry, NULL) != 0) {
                        CPE_ERROR(
                            repo->m_em, "ui_sprite_data_build: set %s(%s): context arg %s read entity id fail!",
                            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), context_arg);
                        return -1;
                    }

                    from_entity = ui_sprite_entity_find_by_id(world, entity_id);
                    if (from_entity == NULL) {
                        CPE_ERROR(
                            repo->m_em, "ui_sprite_data_build: set %s(%s): context entity %d(%s) not exist!",
                            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), entity_id, context_arg);
                        return -1;
                    }
                }
            }
            else {
                uint32_t entity_id;
                char * endp;
                entity_id = (uint32_t)strtol(context_arg, &endp, 10);
                if (endp && *endp == 0) {
                    from_entity = ui_sprite_entity_find_by_id(world, entity_id);
                    if (from_entity == NULL) {
                        CPE_ERROR(
                            repo->m_em, "ui_sprite_data_build: set %s(%s): context entity %d(%s) not exist!",
                            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), entity_id, context_arg);
                        return -1;
                    }
                }
                else {
                    from_entity = ui_sprite_entity_find_by_name(world, context_arg);
                    if (from_entity == NULL) {
                        CPE_ERROR(
                            repo->m_em, "ui_sprite_data_build: set %s(%s): context entity %s not exist!",
                            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), context_arg);
                        return -1;
                    }
                }
            }
        }

        if (from_entity == entity) {
            from_attr = dr_data_entry_search_in_source(&from_attr_buf, data_source, arg_value);
            if (from_attr == NULL) {
                from_attr = ui_sprite_entity_find_attr(&from_attr_buf, from_entity, arg_value);
            }
        }
        else {
            from_attr = ui_sprite_entity_find_attr(&from_attr_buf, from_entity, arg_value);
            if (from_attr == NULL) {
                from_attr = dr_data_entry_search_in_source(&from_attr_buf, data_source, arg_value);
            }
        }

        if (from_attr == NULL) {
            CPE_ERROR(
                repo->m_em, "ui_sprite_data_build: set %s(%s): entry %s not exist!",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), arg_value);
            return -1;
        }

        if (dr_data_entry_set_from_entry(to, from_attr, repo->m_em) != 0) {
            CPE_ERROR(
                repo->m_em, "ui_sprite_data_build: set %s(%s): set from entry fail!",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
            return -1;
        }

        return 0;
    }
    else if (arg_value[0] == 0) {
        if (dr_data_entry_set_from_string(to, arg_value, repo->m_em) != 0) {
            CPE_ERROR(
                repo->m_em, "ui_sprite_data_build: set %s(%s): direct set string fail!",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
            return -1;
        }

        return 0;
    }
    else {
        struct ui_sprite_data_find_ctx ctx = { world, entity, data_source };
        struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
        xtoken_t value;
        int r = -1;

        value = xcomputer_compute(repo->m_computer, arg_value, &calc_args);
        if (value == NULL) {
            CPE_ERROR(
                repo->m_em, "ui_sprite_data_build: set %s(%s) fail!",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
            return -1;
        }

        switch(xtoken_data_type(value)) {
        case xtoken_data_int:
        {
            int64_t v;
            if (xtoken_try_to_int64(value, &v) != 0) {
                CPE_ERROR(
                    repo->m_em, "ui_sprite_data_build: set %s(%s): read int fail!",
                    dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
                break;
            }

            if (dr_entry_set_from_int64(to->m_data, v, to->m_entry, repo->m_em) != 0) {
                CPE_ERROR(
                    repo->m_em, "ui_sprite_data_build: set %s(%s): set int fail!",
                    dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
                break;
            }

            r = 0;
            break;
        }
        case xtoken_data_double:
        {
            double v;
            if (xtoken_try_to_double(value, &v) != 0) {
                CPE_ERROR(
                    repo->m_em, "ui_sprite_data_build: set %s(%s): read double fail!",
                    dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
                break;
            }

            if (dr_entry_set_from_double(to->m_data, v, to->m_entry, repo->m_em) != 0) {
                CPE_ERROR(
                    repo->m_em, "ui_sprite_data_build: set %s(%s): set int fail!",
                    dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
                break;
            }

            r = 0;
            break;
        }
        case xtoken_data_str:
            assert(xtoken_try_to_str(value));
            if (dr_data_entry_set_from_string(to, xtoken_try_to_str(value), repo->m_em) != 0) {
                CPE_ERROR(
                    repo->m_em, "ui_sprite_data_build: set %s(%s): set string fail!",
                    dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
                break;
            }

            r = 0;
            break;
        default:
            CPE_ERROR(
                repo->m_em, "ui_sprite_data_build: set %s(%s): not support type %d!",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), xtoken_data_type(value));
            break;
        }

        xcomputer_free_token(repo->m_computer, value);

        return r;
    }
}
