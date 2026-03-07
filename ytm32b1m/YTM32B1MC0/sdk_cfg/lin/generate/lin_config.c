{{ytCopyright "lin_config.c"}}


#include "lin_config.h"


{{#if this.data.length}}
#define CLASSIC_PID_NUM {{get "pid.length" (itemAt this.data 0) }}
static uint8_t classicPID[CLASSIC_PID_NUM]={ {{get "pid" (itemAt this.data 0) }} };
{{/if}}



{{#each this.data}}
/*{{this.name}}*/

{{#isTruthy this.timerGetTimeIntervalCallback}}
extern uint32_t {{this.timerGetTimeIntervalCallback}}(uint32_t*);
{{/isTruthy}}

lin_state_t {{this.name}}_State;
{{#isTruthy this.readonly}}const {{/isTruthy}}lin_user_config_t {{this.name}} = {
    .baudRate={{this.baudrate}}U,
    .nodeFunction={{this.nodeFunction}},
    .autobaudEnable={{this.autobaudEnable}},
    .timerGetTimeIntervalCallback={{this.timerGetTimeIntervalCallback}},
    .classicPID=classicPID,
    .numOfClassicPID=CLASSIC_PID_NUM,
};
{{/each}}

