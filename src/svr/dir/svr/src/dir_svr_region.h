#ifndef SVR_DIR_SVR_REGION_H
#define SVR_DIR_SVR_REGION_H
#include "dir_svr.h"

struct dir_svr_region {
    dir_svr_t m_svr;
    uint16_t m_region_id;
    char m_region_name[64];
    uint8_t m_region_state;
    uint8_t m_region_type;
    dir_svr_server_list_t m_servers;
    
    uint8_t m_support_chanel_count;
    uint8_t m_support_chanel_capacity;
    char * * m_support_chanels;

    uint8_t m_support_category_count;
    uint8_t m_support_categories[4];
    
    TAILQ_ENTRY(dir_svr_region) m_next;
};

/*region ops*/
dir_svr_region_t dir_svr_region_create(dir_svr_t svr, uint16_t region_id, const char * region_name, uint8_t region_state, uint8_t region_type);
void dir_svr_region_free(dir_svr_region_t region);

uint8_t dir_svr_region_is_support_chanel(dir_svr_region_t region, const char * chanel);
int dir_svr_region_add_support_chanel(dir_svr_region_t region, const char * chanel);

uint8_t dir_svr_region_is_support_category(dir_svr_region_t region, uint8_t device_category);
int dir_svr_region_add_support_category(dir_svr_region_t region, uint8_t device_category);

#endif
