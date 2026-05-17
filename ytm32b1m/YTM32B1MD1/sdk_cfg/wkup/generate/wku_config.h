{{ytCopyright "wku_config.h"}}



#ifndef __WKU_CONFIG_H__
#define __WKU_CONFIG_H__

#include "wku_driver.h"

{{#each this.data}}

#define WKP_PIN_CH_NUM{{@index}} {{this.wkupPins.length}}U
/* {{this.name}} */
extern {{#isTruthy this.readonly}}const {{/isTruthy}}wku_reset_cfg_t {{this.resetName}};
/* Wakeup pins config*/
extern {{#isTruthy this.readonly}}const {{/isTruthy}}wku_pin_wakeup_cfg_t {{this.pinsName}}[{{this.wkupPins.length}}];
{{/each}}

#endif


