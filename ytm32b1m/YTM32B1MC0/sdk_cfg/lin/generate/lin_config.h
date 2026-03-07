{{ytCopyright "lin_config.h"}}



#ifndef __LIN_CONFIG_H__
#define __LIN_CONFIG_H__




#include "lin_driver.h"


{{#each this.data}}
/*{{this.name}}*/
extern lin_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}}lin_user_config_t {{this.name}};
{{/each}}





#endif

