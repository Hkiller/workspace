#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/string_utils.h"
#include "plugin_chipmunk_env_i.h"
#include "plugin_chipmunk_env_updator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_env_t plugin_chipmunk_env_create(plugin_chipmunk_module_t module) {
    plugin_chipmunk_env_t env;

    env = (plugin_chipmunk_env_t)mem_alloc(module->m_alloc, sizeof(struct plugin_chipmunk_env));
    if (env == NULL) {
        CPE_ERROR(module->m_em, "plugin_chipmunk_env_create: create fail!");
        return NULL;
    }

    bzero(env, sizeof(*env));
    
    env->m_module = module;
    env->m_left_time = 0.0f;
    env->m_ptm = 1.0f;
    env->m_time_scale = 1.0f;
    env->m_step_duration = 1.0f / 60.0f;

    if (cpSpaceInit(&env->m_space) == NULL) {
        CPE_ERROR(module->m_em, "plugin_chipmunk_env_create: create space fail!");
        mem_free(module->m_alloc, env);
        return NULL;
    }

    TAILQ_INIT(&env->m_updators);
    
    cpSpaceSetUserData(&env->m_space, env);
	cpSpaceSetIterations(&env->m_space, 10);
	cpSpaceSetSleepTimeThreshold(&env->m_space, 0.5f);

    return env;
}

void plugin_chipmunk_env_free(plugin_chipmunk_env_t env) {
    plugin_chipmunk_module_t module = env->m_module;

    assert(env->m_space.dynamicBodies->num == 0);
    assert(env->m_space.staticBodies->num == 0);
    assert(TAILQ_EMPTY(&env->m_updators));

    cpSpaceDestroy(&env->m_space);

    mem_free(module->m_alloc, env);
}

int plugin_chipmunk_env_update(plugin_chipmunk_env_t env, float delta) {
    plugin_chipmunk_env_updator_t updator;

    delta *= env->m_time_scale;
    env->m_left_time += delta;

    while(env->m_left_time >= env->m_step_duration) {

        TAILQ_FOREACH(updator, &env->m_updators, m_next) {
            updator->m_fun(env, updator->m_ctx, env->m_step_duration);
        }
        
        cpSpaceStep(&env->m_space, env->m_step_duration);
        env->m_left_time -= env->m_step_duration;
    }

    return 0;
}

void * plugin_chipmunk_env_space(plugin_chipmunk_env_t env) {
    return &env->m_space;
}

float plugin_chipmunk_env_ptm(plugin_chipmunk_env_t env) {
    return env->m_ptm;
}

void plugin_chipmunk_env_set_ptm(plugin_chipmunk_env_t env, float ptm) {
    env->m_ptm = ptm;
}

float plugin_chipmunk_env_time_scale(plugin_chipmunk_env_t env) {
    return env->m_time_scale;
}
    
void plugin_chipmunk_env_set_time_scale(plugin_chipmunk_env_t env, float time_scale) {
    env->m_time_scale = time_scale;
}
    
float plugin_chipmunk_env_step_duration(plugin_chipmunk_env_t env) {
    return env->m_step_duration;
}

void plugin_chipmunk_env_set_step_duration(plugin_chipmunk_env_t env, float step_duration) {
    env->m_step_duration = step_duration;
}

void plugin_chipmunk_env_set_gravity(plugin_chipmunk_env_t env, float gravity_x, float gravity_y) {
    cpSpaceSetGravity(&env->m_space, cpv(gravity_x * env->m_ptm, gravity_y * env->m_ptm));
}

void plugin_chipmunk_env_set_spatial_hash(plugin_chipmunk_env_t env, float dim, uint32_t count) {
    cpSpaceUseSpatialHash(&env->m_space, dim, (int)count);
}

int plugin_chipmunk_env_register_collision_type(uint32_t * type, plugin_chipmunk_env_t env, const char * collision_type_name) {
    uint32_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(env->m_collision_types); ++i) {
        if (env->m_collision_types[i].m_name[0] == 0) {
            cpe_str_dup(env->m_collision_types[i].m_name, sizeof(env->m_collision_types[i].m_name), collision_type_name);
            *type = ((uint32_t)1) << i;
            return 0;
        }
        else if (strcmp(env->m_collision_types[i].m_name, collision_type_name) == 0) {
            *type = ((uint32_t)1) << i;
            return 0;
        }
    }

    return -1;
}
    
int plugin_chipmunk_env_mask_register(plugin_chipmunk_env_t env, const char * mask_name) {
    uint32_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(env->m_mask); ++i) {
        if (env->m_mask[i].m_name[0] == 0) {
            cpe_str_dup(env->m_mask[i].m_name, sizeof(env->m_mask[i].m_name), mask_name);
            return 0;
        }
    }

    return -1;
}

void plugin_chipmunk_env_mask_unregister(plugin_chipmunk_env_t env, const char * laye_name) {
    uint32_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(env->m_mask); ++i) {
        if (strcmp(env->m_mask[i].m_name, laye_name) == 0) {
            env->m_mask[i].m_name[0] = 0;
            break;
        }
    }
}

uint32_t plugin_chipmunk_env_mask(plugin_chipmunk_env_t env, const char * laye_name) {
    uint32_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(env->m_mask); ++i) {
        if (strcmp(env->m_mask[i].m_name, laye_name) == 0) return ((uint32_t)1) << i;
    }

    return 0;
}

int plugin_chipmunk_env_masks(plugin_chipmunk_env_t env, uint32_t * masks, const char * laye_name) {    
    const char * sep;
    uint32_t v;

    * masks = 0;

    while((sep = strchr(laye_name, ':'))) {
        char buf[64];
        size_t len = sep - laye_name;

        if (len + 1 > CPE_ARRAY_SIZE(buf)) return -1;

        memcpy(buf, laye_name, len);
        buf[len] = 0;

        v = plugin_chipmunk_env_mask(env, buf);
        if (v == 0) return -1;

        *masks |= v;
        laye_name = sep + 1;
    }

    v = plugin_chipmunk_env_mask(env, laye_name);
    if (v == 0) return -1;
    *masks |= v;

    return 0;
}
    
#ifdef __cplusplus
}
#endif
