{{ytCopyright "wdg_config.h"}}



#ifndef __WDG_CONFIG_H__
#define __WDG_CONFIG_H__




#include "wdg_driver.h"



{{#each this.data}}
/*{{this.name}}*/
extern {{#isTruthy this.readonly}}const {{/isTruthy}}wdg_user_config_t {{this.name}};
{{/each}}





#endif

