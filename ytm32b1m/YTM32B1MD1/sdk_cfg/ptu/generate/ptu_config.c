{{ytCopyright "ptu_config.c"}}


#include "ptu_config.h"


{{#each this.data}}
/*{{this.name}}*/
{{#isTruthy this.readonly}}const {{/isTruthy}}ptu_timer_config_t {{this.name}} = {
    .loadValueMode={{this.loadValueMode}},
    .clkPreDiv={{this.clkPreDiv}},
    .clkPreMultFactor={{this.clkPreMultFactor}},
    .triggerInput={{this.triggerInput}},
    .continuousModeEnable={{this.continuousModeEnable}},
    .dmaEnable={{this.dmaEnable}},
    .intEnable={{this.intEnable}},
};
{{/each}}

