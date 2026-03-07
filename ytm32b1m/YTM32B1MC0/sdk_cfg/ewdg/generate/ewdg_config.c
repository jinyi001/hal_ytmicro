{{ytCopyright "ewdg_config.c"}}


#include "ewdg_config.h"


{{#each this.data}}
/*{{this.name}}*/
{{#isTruthy this.readonly}}const {{/isTruthy}}ewdg_init_config_t {{this.name}} = {
{{#if (getGlobalVar 'meta.ewdg_clk_sel_en')}}
    .clockSource={{this.clockSource}},
{{/if}}
    .assertLogic={{this.assertLogic}},
    .interruptEnable={{this.interruptEnable}},
    .prescaler={{this.prescaler}},
    .compareLow={{this.compareLow}},
    .compareHigh={{this.compareHigh}},
};
{{/each}}

