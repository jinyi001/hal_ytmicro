{{ytCopyright "i2c_config.c"}}

#include <stddef.h>
#include "i2c_config.h"

/* I2C master */
{{#each this.data.master}}
/*{{this.name}}*/
{{#isTruthy this.masterCallback}}extern void {{{this.masterCallback}}}(i2c_master_event_t event, void *userData);{{/isTruthy}}
{{#isTruthy this.callbackParam}}extern void* {{{this.callbackParam}}};{{/isTruthy}}

i2c_master_state_t {{this.name}}_State;
{{#isTruthy this.readonly}}const {{/isTruthy}}i2c_master_user_config_t {{this.name}} = {
    .slaveAddress={{this.slaveAddress}}U,
    .is10bitAddr={{this.is10bitAddr}},
    .operatingMode={{this.operatingMode}},
    .transferType={{this.transferType}},
    .baudRate={{this.baudRate}},
    .dmaChannel={{this.dmaChannel}},
    .masterCallback={{{this.masterCallback}}},
    .callbackParam={{{this.callbackParam}}},
};
{{/each}}
/* I2C slave */
{{#each this.data.slave}}
/*{{this.name}}*/
i2c_slave_state_t {{this.name}}_State;
{{#isTruthy this.slaveCallback}}extern void {{{this.slaveCallback}}}(i2c_slave_event_t event, void *userData);{{/isTruthy}}
{{#isTruthy this.callbackParam}}extern void* {{{this.callbackParam}}};{{/isTruthy}}

{{#isTruthy this.readonly}}const {{/isTruthy}}i2c_slave_user_config_t {{this.name}} = {
    .slaveAddress={{this.slaveAddress}}U,
    .is10bitAddr={{this.is10bitAddr}},
    .operatingMode={{this.operatingMode}},
    .slaveListening={{this.slaveListening}},
    .transferType={{this.transferType}},
    .dmaChannel={{this.dmaChannel}},
    .slaveCallback={{{this.slaveCallback}}},
    .callbackParam={{{this.callbackParam}}},
};
{{/each}}

