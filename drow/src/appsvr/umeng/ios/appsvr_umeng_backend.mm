#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "appsvr_umeng_backend.h"
#include "../appsvr_umeng_executor.h"

int appsvr_umeng_backend_init(appsvr_umeng_module_t module) {
    [MobClick startWithAppkey: [[NSString alloc] initWithCString: module->m_app_key] 
                 reportPolicy: BATCH
                    channelId: [[NSString alloc] initWithCString: cfg_get_string(gd_app_cfg(module->m_app), "umeng-chanle.ios", "")] ];
    return 0;
}

void appsvr_umeng_backend_fini(appsvr_umeng_module_t module) {
}

void appsvr_umeng_on_page_begin(appsvr_umeng_module_t, const char * page_name) {
}

void appsvr_umeng_on_page_end(appsvr_umeng_module_t, const char * page_name) {
}

void appsvr_umeng_on_pause(appsvr_umeng_module_t module) {
}

void appsvr_umeng_on_resume(appsvr_umeng_module_t module) {
}

void appsvr_umeng_on_event(appsvr_umeng_module_t module, const char * str_id, uint32_t count, const char * str_attrs) {
}

int appsvr_umeng_executor_backend_init(appsvr_umeng_executor_t executor) {
    return 0;
}

void appsvr_umeng_executor_backend_exec(appsvr_umeng_executor_t executor, dr_data_source_t data_source) {
    appsvr_umeng_module_t module = executor->m_module;
    appsvr_umeng_backend_t backend = module->m_backend;
    
    if (strcmp(executor->m_op_name, "pay") == 0) {
        assert(executor->m_arg_count == 5);
        const char * item = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[1].m_def, data_source, "");
        if (item[0]) {
            [MobClickGameAnalytics pay : (double)dr_calc_double_with_dft(module->m_computer, executor->m_args[0].m_def, data_source, 0.0)
                                source : (int)dr_calc_int64_with_dft(module->m_computer, executor->m_args[4].m_def, data_source, 0)
                                  item : [[NSString alloc] initWithCString: item]
                                amount : (int)dr_calc_int64_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0)
                                 price : (double)dr_calc_double_with_dft(module->m_computer, executor->m_args[3].m_def, data_source, 0.0)
                ];
        }
        else {
            [MobClickGameAnalytics pay : (double)dr_calc_double_with_dft(module->m_computer, executor->m_args[0].m_def, data_source, 0.0)
                                source : (int)dr_calc_int64_with_dft(module->m_computer, executor->m_args[4].m_def, data_source, 0)
                                amount : (int)dr_calc_int64_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0)
                ];
        }
    }
    else if (strcmp(executor->m_op_name, "buy") == 0) {
        assert(executor->m_arg_count == 3);
        const char * item = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "");
        [MobClickGameAnalytics buy : [[NSString alloc] initWithCString: item]
                             amount : (int)dr_calc_int64_with_dft(module->m_computer, executor->m_args[1].m_def, data_source, 0)
                              price : (double)dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0)
                ];
    }
    else if (strcmp(executor->m_op_name, "use") == 0) {
        assert(executor->m_arg_count == 3);
        const char * item = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "");
        [MobClickGameAnalytics use : [[NSString alloc] initWithCString: item]
                             amount : (int)dr_calc_int64_with_dft(module->m_computer, executor->m_args[1].m_def, data_source, 0)
                              price : (double)dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0)
                ];
    }
    else if (strcmp(executor->m_op_name, "startLevel") == 0) {
        assert(executor->m_arg_count == 1);
        const char * level = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "");
        [MobClickGameAnalytics startLevel : [[NSString alloc] initWithCString: level]];
    }
    else if (strcmp(executor->m_op_name, "finishLevel") == 0) {
        assert(executor->m_arg_count == 1);
        const char * level = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "");
        [MobClickGameAnalytics finishLevel : [[NSString alloc] initWithCString: level]];
    }
    else if (strcmp(executor->m_op_name, "failLevel") == 0) {
        assert(executor->m_arg_count == 1);
        const char * level = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "");
        [MobClickGameAnalytics failLevel : [[NSString alloc] initWithCString: level]];
    }
    else if (strcmp(executor->m_op_name, "bonus") == 0) {
        const char * item;

        assert(executor->m_arg_count == 4);

        item = dr_calc_str_with_dft(&module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "");
        if (item[0]) {
            [MobClickGameAnalytics bonus : [[NSString alloc] initWithCString: item]
                                  amount : dr_calc_int64_with_dft(module->m_computer, executor->m_args[1].m_def, data_source, 0)
                                   price : dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0)
                                  source : dr_calc_int64_with_dft(module->m_computer, executor->m_args[3].m_def, data_source, 0)
                ];
        }
        else {
            [MobClickGameAnalytics bonus : dr_calc_double_with_dft(module->m_computer, executor->m_args[2].m_def, data_source, 0.0)
                                  source : dr_calc_int64_with_dft(module->m_computer, executor->m_args[3].m_def, data_source, 0)
                ];
        }
    }
    else if (strcmp(executor->m_op_name, "onProfileSignIn") == 0) {
        assert(executor->m_arg_count == 2);
        
        NSString * puid = [[NSString alloc]
                              initWithCString : dr_calc_str_with_dft(
                                  &module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "")];

        NSString * provider = [[NSString alloc]
                              initWithCString : dr_calc_str_with_dft(
                                  &module->m_dump_buffer, module->m_computer, executor->m_args[1].m_def, data_source, "")];
        
        [MobClickGameAnalytics
            profileSignInWithPUID : puid
                         provider : provider
            ];
    }
    else if (strcmp(executor->m_op_name, "onProfileSignOff") == 0) {
        assert(executor->m_arg_count == 0);
        [MobClickGameAnalytics profileSignOff];
    }
    else if (strcmp(executor->m_op_name, "startLevel") == 0) {
        assert(executor->m_arg_count == 1);
        
        [MobClickGameAnalytics setUserLevelId: dr_calc_int64_with_dft(module->m_computer, executor->m_args[0].m_def, data_source, 0)];
    }
    else if (strcmp(executor->m_op_name, "onEvent") == 0){
        NSString * id = [[NSString alloc]
                           initWithCString : dr_calc_str_with_dft(
                               &module->m_dump_buffer, module->m_computer, executor->m_args[0].m_def, data_source, "")];
        [MobClick event : id];
    }
    else {
        CPE_ERROR(module->m_em, "umeng: call %s not support!", executor->m_op_name);
        return;
    }

    CPE_INFO(module->m_em, "umeng: %s ==> %s success!", dr_meta_name(data_source->m_data.m_meta), executor->m_op_name);
}

void appsvr_umeng_executor_backend_fini(appsvr_umeng_executor_t executor) {
}
