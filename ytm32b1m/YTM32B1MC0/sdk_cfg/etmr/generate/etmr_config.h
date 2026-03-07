{{ytCopyright "etmr_config.h"}}



#ifndef __ETMR_CONFIG_H__
#define __ETMR_CONFIG_H__



#include "etmr_common.h"
{{#if this.data.mc.enable}}#include "etmr_mc_driver.h"{{/if}}
{{#if this.data.oc.enable}}#include "etmr_oc_driver.h"{{/if}}
{{#if this.data.pwm.enable}}#include "etmr_pwm_driver.h"{{/if}}
{{#if this.data.qd.enable}}#include "etmr_qd_driver.h"{{/if}}
{{#if this.data.ic.enable}}#include "etmr_ic_driver.h"{{/if}}



/*
 * Common
*/
{{#each this.data.common}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}etmr_user_config_t {{this.name}};
extern etmr_state_t {{this.name}}_State;
{{/each}}


/*
 * MC
*/
{{#if this.data.mc.enable}}
{{#each this.data.mc.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}etmr_timer_param_t {{this.name}};
{{/each}}
{{/if}}


/*
 * PWM
*/
{{#if this.data.pwm.enable}}
{{#each this.data.pwm.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}etmr_pwm_param_t {{this.name}};
{{/each}}
{{/if}}


/*
 * IC
*/
{{#if this.data.ic.enable}}
{{#each this.data.ic.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}etmr_ic_param_t {{this.name}};
{{/each}}
{{/if}}


/*
 * OC
*/
{{#if this.data.oc.enable}}
{{#each this.data.oc.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}etmr_oc_param_t {{this.name}};
{{/each}}
{{/if}}



/*
 * QD
*/
{{#if this.data.qd.enable}}
{{#each this.data.qd.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}etmr_qd_config_t {{this.name}};
extern etmr_qd_state_t {{this.name}}_State;
{{/each}}
{{/if}}

#endif


