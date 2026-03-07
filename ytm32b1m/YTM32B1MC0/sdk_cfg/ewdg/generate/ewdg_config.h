{{ytCopyright "ewdg_config.h"}}



#ifndef __EWDG_CONFIG_H__
#define __EWDG_CONFIG_H__




#include "ewdg_driver.h"



{{#each this.data}}
/*{{this.name}}*/
extern {{#isTruthy this.readonly}}const {{/isTruthy}}ewdg_init_config_t {{this.name}};
{{/each}}





#endif

