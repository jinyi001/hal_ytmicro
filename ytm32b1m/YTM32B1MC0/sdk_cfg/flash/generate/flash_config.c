{{ytCopyright "flash_config.c"}}

#include <stddef.h>
#include "flash_config.h"


{{#each this.data.config}}
/*{{this.name}}*/

flash_state_t {{this.name}}_State;

{{#isTruthy this.callback}}extern void {{{this.callback}}}(flash_event_t event);{{/isTruthy}}

{{#isTruthy this.readonly}}const {{/isTruthy}}flash_user_config_t {{this.name}}={
    .async={{this.async}},
    .disGlobalInt={{this.disGlobalInt}},
    .readVerify={{this.readVerify}},
    .callback={{this.callback}},
};
{{/each}}

