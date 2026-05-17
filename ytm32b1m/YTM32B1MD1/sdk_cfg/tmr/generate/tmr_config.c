{{ytCopyright "tmr_config.c"}}

#include "tmr_config.h"

{{#each this.data.channel}}
const tmr_channel_config_t tmr_channel_{{this.channel}}={
    .channel={{this.channel}},
    .compareValue={{this.compareValue}},
{{#if (getGlobalVar 'meta.tmr_has_channel_int')}}
    .interruptEnable={{this.intEn}},
{{/if}}
};
{{/each}}

{{#isTruthy this.data.readonly}}const {{/isTruthy}}tmr_config_t {{this.data.name}}={
    .clockPrescaler={{this.data.clockPrescaler}},
    .stopInDebugMode={{this.data.stopInDebugMode}},
    .startValue={{this.data.startValue}},
};

