{{ytCopyright "wdg_config.c"}}


#include "wdg_config.h"


{{#each this.data}}
/*{{this.name}}*/
{{#isTruthy this.readonly}}const {{/isTruthy}}wdg_user_config_t {{this.name}} = {
    .clockSource={{this.clockSource}},
    .opMode={
        .deepsleep={{this.opMode.deepsleep}},
        .debug={{this.opMode.debug}},
    },
    .updateEnable={{this.updateEnable}},
    .intEnable={{this.intEnable}},
    .winEnable={{this.winEnable}},
    .windowValue={{this.windowValue}},
    .timeoutValue={{this.timeoutValue}},
    .apbErrorResetEnable={{this.apbErrorResetEnable}},
};
{{/each}}

