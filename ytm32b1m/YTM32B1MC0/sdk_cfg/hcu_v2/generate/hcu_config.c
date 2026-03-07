{{ytCopyright "hcu_config.c"}}

#include "hcu_config.h"





{{#each this.data}}
/* {{this.name}} */
hcu_state_t {{this.name}}_State;
{{#isTruthy this.readonly}}const {{/isTruthy}}hcu_user_config_t {{this.name}}={
    .swap={{this.swap}},
    .carryType={{this.carryType}},
    .ingressDMAChannel={{this.ingressDMAChannel}},
    .egressDMAChannel={{this.egressDMAChannel}},
};
{{/each}}


