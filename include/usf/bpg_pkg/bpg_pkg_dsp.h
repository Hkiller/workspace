#ifndef USF_BPG_PKG_DSP_H
#define USF_BPG_PKG_DSP_H
#include "bpg_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_pkg_dsp_t bpg_pkg_dsp_create(mem_allocrator_t alloc);
void bpg_pkg_dsp_free(bpg_pkg_dsp_t dsp);

int bpg_pkg_dsp_load(bpg_pkg_dsp_t dsp, cfg_t cfg, error_monitor_t em);
int bpg_pkg_dsp_dispatch(bpg_pkg_dsp_t dsp, dp_req_t body, error_monitor_t em);
int bpg_pkg_dsp_pass(bpg_pkg_dsp_t dsp, dp_req_t req, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
