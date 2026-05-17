{{ytCopyright "wku_config.c"}}

#include "wku_config.h"





{{#each this.data}}
/* {{this.name}} */
/* Reset config*/
{{#isTruthy this.readonly}}const {{/isTruthy}}wku_reset_cfg_t {{this.resetName}}={
    .wakeupEn={{this.wakeupEn}},
    .filterEn={{this.filterEn}},
    {{#isTruthy (getGlobalVar 'meta.FEATURE_WKU_FILTER_CLK_SRC')}}
    .filterClkSrc={{this.filterClkSrc}},
    {{/isTruthy}}
};
/* Wakeup pins config*/
{{#isTruthy this.readonly}}const {{/isTruthy}}wku_pin_wakeup_cfg_t {{this.pinsName}}[WKP_PIN_CH_NUM{{@index}}]={
    {{#each this.wkupPins}}
    {
        .hwChannel={{this.hwChannel}},
        .wakeupEn={{this.wakeupEn}},
        .edgeEvent={{this.edgeEvent}},
        .filterEn={{this.filterEn}},
        {{#isTruthy (getGlobalVar 'meta.FEATURE_WKU_FILTER_CLK_SRC')}}
        .filterClkSrc={{this.filterClkSrc}},
        {{/isTruthy}}
    },
    {{/each}}
};
{{/each}}


