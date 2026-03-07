{{ytCopyright "tmu_config.c"}}

#include "tmu_config.h"

const tmu_inout_mapping_config_t inOutMappingConfig[{{this.data.length}}]={
{{#each this.data}}
    {
        .triggerSource={{this.triggerSource}},
        .targetModule={{this.targetModule}}
    },
{{/each}}
};

const tmu_user_config_t tmu_config0={
    .numInOutMappingConfigs={{this.data.length}},
    .inOutMappingConfig=inOutMappingConfig,
};

