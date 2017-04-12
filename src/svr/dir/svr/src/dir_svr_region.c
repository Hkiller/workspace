#include "cpe/utils/string_utils.h"
#include "dir_svr_region.h"
#include "dir_svr_server.h"

dir_svr_region_t
dir_svr_region_create(dir_svr_t svr, uint16_t region_id, const char * region_name, uint8_t region_state, uint8_t region_type) {
    dir_svr_region_t region;

    region = mem_alloc(svr->m_alloc, sizeof(struct dir_svr_region));
    if (region == NULL) return NULL;

    region->m_svr = svr;
    region->m_region_id = region_id;
    cpe_str_dup(region->m_region_name, sizeof(region->m_region_name), region_name);
    region->m_region_state = region_state;
    region->m_region_type = region_type;
    region->m_support_chanel_capacity = 0;
    region->m_support_chanel_count = 0;
    region->m_support_chanels = NULL;
    region->m_support_category_count = 0;
    TAILQ_INIT(&region->m_servers);

    svr->m_region_count++;
    TAILQ_INSERT_TAIL(&svr->m_regions, region, m_next);

    return region;
}

void dir_svr_region_free(dir_svr_region_t region) {
    dir_svr_t svr = region->m_svr;
    uint8_t i;

    for(i = 0; i < region->m_support_chanel_count; ++i) {
        mem_free(svr->m_alloc, region->m_support_chanels[i]);
    }
    
    if (region->m_support_chanels) {
        mem_free(svr->m_alloc, region->m_support_chanels);
    }
    
    while(!TAILQ_EMPTY(&region->m_servers)) {
        dir_svr_server_free(TAILQ_FIRST(&region->m_servers));
    }

    svr->m_region_count--;
    TAILQ_REMOVE(&svr->m_regions, region, m_next);
    mem_free(svr->m_alloc, region);
}

uint8_t dir_svr_region_is_support_chanel(dir_svr_region_t region, const char * chanel) {
    uint8_t i;

    for(i = 0; i < region->m_support_chanel_count; ++i) {
        if (strcmp(region->m_support_chanels[i], chanel) == 0) return 1;
    }
    
    return 0;
}

int dir_svr_region_add_support_chanel(dir_svr_region_t region, const char * chanel) {
    dir_svr_t svr = region->m_svr;
    char * new_chanel;
    
    if (region->m_support_chanel_count + 1 > region->m_support_chanel_capacity) {
        uint32_t new_capacity;
        char ** new_buff;
        
        new_capacity = region->m_support_chanel_capacity < 32 ? 32 : region->m_support_chanel_capacity * 2;
        if (new_capacity > 256) new_capacity = 256;

        if (new_capacity == region->m_support_chanel_capacity) {
            CPE_ERROR(svr->m_em, "region support chanel max reached, capacity=%d", region->m_support_chanel_capacity);
            return -1;
        }

        new_buff = mem_alloc(svr->m_alloc, sizeof(char*) * new_capacity);
        if (new_buff == NULL) {
            CPE_ERROR(svr->m_em, "region alloc chanel buf fail, capacity=%d", new_capacity);
            return -1;
        }

        if (region->m_support_chanel_count) {
            memcpy(new_buff, region->m_support_chanels, sizeof(char*) * region->m_support_chanel_count);
        }

        if (region->m_support_chanels) mem_free(svr->m_alloc, region->m_support_chanels);

        region->m_support_chanels = new_buff;
        region->m_support_chanel_capacity = new_capacity;
    }

    new_chanel = cpe_str_mem_dup(svr->m_alloc, chanel);
    if (new_chanel == NULL) {
        CPE_ERROR(svr->m_em, "region alloc chanel fail");
        return -1;
    }

    region->m_support_chanels[region->m_support_chanel_count++] = new_chanel;
    return 0;
}

uint8_t dir_svr_region_is_support_category(dir_svr_region_t region, uint8_t device_category) {
    uint8_t i;

    for(i = 0; i < region->m_support_category_count; ++i) {
        if (region->m_support_categories[i] == device_category) return 1;
    }
    
    return 0;
}

int dir_svr_region_add_support_category(dir_svr_region_t region, uint8_t device_category) {
    dir_svr_t svr = region->m_svr;
    
    if (region->m_support_category_count + 1 > CPE_ARRAY_SIZE(region->m_support_categories)) {
        CPE_ERROR(svr->m_em, "region %s support categories count overflow", region->m_region_name);
        return -1;
    }

    region->m_support_categories[region->m_support_category_count++] = device_category;
    
    return 0;
}
