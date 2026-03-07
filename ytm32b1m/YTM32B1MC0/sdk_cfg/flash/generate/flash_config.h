{{ytCopyright "flash_config.h"}}



#ifndef __FLASH_CONFIG_H__
#define __FLASH_CONFIG_H__




#include "flash_driver.h"


{{#if this.data.custAddr}}
{{#each this.data.cusInfo}}
#define {{this.name}}_CUSTOM_ADDR    (0x{{nToString this.start 16}}U)
#define {{this.name}}_CUSTOM_SIZE    (0x{{nToString this.size 16}}U)
{{/each}}
{{/if}}


{{#each this.data.config}}
/*{{this.name}}*/
extern flash_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}}flash_user_config_t {{this.name}};
{{/each}}

#endif

