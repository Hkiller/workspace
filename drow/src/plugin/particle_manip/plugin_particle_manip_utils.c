#include "cpe/pal/pal_string.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin_particle_manip_utils.h"

static struct {
    const char * type_name;
    uint32_t type_hash;
} s_particle_mod_defs[] = {
    /* 1*/ { "RParticleAttractAccelMOD", 0x2794b9ef },
    /* 2*/ { "RParticleDampingAccelMOD", 0xfd70fb4d },
    /* 3*/ { "RParticleSeedAccelMOD", 0xABCDD4B8 },
    /* 4*/ { "RParticleSineAccelMOD", 0x383f6c77 },
    /* 5*/ { "RParticleCurvedColorMOD", 0x85fab0a2 },
    /* 6*/ { "RParticleCurvedAlphaMOD", 0xc82a0e7b },
    /* 7*/ { "RParticleFixedColorMOD", 0xC1368471 },
    /* 8*/ { "RParticleOverLifeColorMOD", 0x27453D9B },
    /* 9*/ { "RParticleSeedColorMOD", 0xFB12331C },
    /*10*/ { "RParticleSeedLifetimeMOD", 0x14474AFD },
    /*11*/ { "RParticleOrbitLocationMOD", 0xd4f63e01 },
    /*12*/ { "RParticleSeedLocationMOD", 0x29BAB14E },
    /*13*/ { "RParticleSeedRotation2DMOD", 0x61334946 },
    /*14*/ { "RParticleCurvedUniformSizeMOD", 0x85e88758},
    /*15*/ { "RParticleCurvedSizeMOD", 0x51916d2f },
    /*16*/ { "RParticleUniformOverLifeSizeMOD", 0xA36FC378 },
    /*17*/ { "RParticleOverLifeSizeMOD", 0xd42b5a2a },
    /*18*/ { "RParticleSeedSizeMOD", 0x8FC2AA86 },
    /*19*/ { "RParticleUniformSeedSizeMOD", 0x13CB2444 },
    /*20*/ { "RParticleFlipbookUVMOD", 0x4bb9a385 },
    /*21*/ { "RParticleScrollAnimMOD", 0x03850cb0 },
    /*22*/ { "RParticleTileSubTexMOD", 0x67e0030c },
    /*23*/ { "RParticleCircleSpawnMOD", 0x85849DEA },
    /*24*/ { "RParticleEllipseSpawnMod", 0xbf3738cf },
    /*25*/ { "RParticleAttractVelocityMOD", 0xbeeed874 },
    /*26*/ { "RParticleSeedVelocityMOD", 0xBB55B0F7 },
    /*27*/ { "RParticleHorizontalStopMOD", 0x6629e4c8 }
};

const char * plugin_particle_manip_proj_particle_mod_type_name(uint8_t mod_type) {
    if (mod_type < 1 || (mod_type - 1) >= CPE_ARRAY_SIZE(s_particle_mod_defs)) {
        return "unknown-mode-type";
    }

    return s_particle_mod_defs[mod_type - 1].type_name;
}

uint32_t plugin_particle_manip_proj_particle_mod_type_hash(uint8_t mod_type) {
    if (mod_type < 1 || (mod_type - 1) >= CPE_ARRAY_SIZE(s_particle_mod_defs)) {
        return 0;
    }

    return s_particle_mod_defs[mod_type - 1].type_hash;
}

uint8_t plugin_particle_manip_proj_particle_mod_type(const char * mod_type_name) {
    uint8_t i;
    for(i = 0; i < CPE_ARRAY_SIZE(s_particle_mod_defs); ++i) {
        if (strcmp(s_particle_mod_defs[i].type_name, mod_type_name) == 0) {
            return i + 1;
        }
    }

    return 0;
}
