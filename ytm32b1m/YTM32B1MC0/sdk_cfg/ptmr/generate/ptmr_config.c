{{ytCopyright "ptmr_config.c"}}

#include "ptmr_config.h"

{{#each this.data.channel}}
const ptmr_user_channel_config_t ptmr_channel_{{this.channel}}={
    .periodUnits={{this.periodUnits}},
    .period={{this.period}},
    .chainChannel={{this.chainChannel}},
    .isInterruptEnabled={{this.isInterruptEnabled}},
};
{{/each}}

{{#isTruthy this.data.readonly}}const {{/isTruthy}}ptmr_user_config_t {{this.data.name}}={
    .enableRunInDebug={{this.data.enableRunInDebug}},
{{#if (getGlobalVar 'meta.FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE')}}
    .useFunctionClockSource={{this.data.useFunctionClockSource}},
{{/if}}
};

