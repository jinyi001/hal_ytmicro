{{ytCopyright "can_config.h"}}



#ifndef __CAN_CONFIG_H__
#define __CAN_CONFIG_H__




#include "flexcan_driver.h"


{{#each this.data}}
#define {{this.name}}_INST {{this.inst}}
{{/each}}


{{#each this.data}}
extern flexcan_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}} flexcan_user_config_t  {{this.name}};


{{/each}}





#endif

