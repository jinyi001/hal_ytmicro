{{ytCopyright "hcu_config.h"}}



#ifndef __HCU_CONFIG_H__
#define __HCU_CONFIG_H__

#include "hcu_driver.h"

{{#each this.data}}
/* {{this.name}} */
extern hcu_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}}hcu_user_config_t {{this.name}};
{{/each}}

#endif


