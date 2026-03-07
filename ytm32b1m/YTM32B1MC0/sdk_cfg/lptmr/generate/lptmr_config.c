{{ytCopyright "ltmr_config.c"}}

#include "lptmr_config.h"

{{#isTruthy this.data.readonly}}const {{/isTruthy}}lptmr_config_t {{this.data.name}}={
{{#if (getGlobalVar 'meta.dma_num')}}
    .dmaRequest={{this.data.dmaRequest}},
{{/if}}
    .interruptEnable={{this.data.interruptEnable}},
    .freeRun={{this.data.freeRun}},
    .workMode={{this.data.workMode}},
    .prescaler={{this.data.prescaler}},
    .bypassPrescaler={{this.data.bypassPrescaler}},
    .compareValue={{this.data.compareValue}},
    .counterUnits={{this.data.counterUnits}},
    .pinSelect={{this.data.pinSelect}},
    .pinPolarity={{this.data.pinPolarity}},
{{#if (getGlobalVar 'meta.FEATURE_LPTMR_CLKSRC_SELECT_ENABLE')}}
    .clockSource={{this.data.clockSource}},
{{/if}}
};

