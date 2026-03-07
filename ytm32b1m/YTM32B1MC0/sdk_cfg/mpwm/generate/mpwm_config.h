{{ytCopyright "mpwm_config.h"}}



#ifndef __MPWM_CONFIG_H__
#define __MPWM_CONFIG_H__



#include "mpwm_common.h"
{{#if this.data.ct.enable}}#include "mpwm_ct_driver.h"{{/if}}
{{#if this.data.pwm.enable}}#include "mpwm_pwm_driver.h"{{/if}}
{{#if this.data.oc.enable}}#include "mpwm_oc_driver.h"{{/if}}
{{#if this.data.ic.enable}}#include "mpwm_ic_driver.h"{{/if}}
{{#if this.data.pc.enable}}#include "mpwm_pc_driver.h"{{/if}}

/*
 * Common
*/
{{#each this.data.common}}
extern mpwm_state_t {{this.state_name}};
{{#if (getGlobalVar 'meta.FEATURE_MPWM_HAS_IPC_CLOCK_SOURCE')}}
extern const mpwm_user_config_t {{this.name}};
{{/if}}
{{/each}}

/*
 * Common timer
*/
{{#if this.data.ct.enable}}
{{#each this.data.ct.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_ct_config_t {{this.name}};
{{/each}}
{{/if}}

/*
 * PWM
*/
{{#if this.data.pwm.enable}}
{{#each this.data.pwm.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_pwm_config_t {{this.name}};
{{/each}}
{{/if}}

/*
 * Outout compare
*/
{{#if this.data.oc.enable}}
{{#each this.data.oc.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_oc_config_t {{this.name}};
{{/each}}
{{/if}}

/*
 * Input capture
*/
{{#if this.data.ic.enable}}
{{#each this.data.ic.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_ic_config_t {{this.name}};
{{/each}}
{{/if}}

/*
 * Pulse count
*/
{{#if this.data.pc.enable}}
{{#each this.data.pc.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_pc_config_t {{this.name}};
{{/each}}
{{/if}}

#endif


