{{ytCopyright "ptu_config.h"}}



#ifndef __PTU_CONFIG_H__
#define __PTU_CONFIG_H__




#include "ptu_driver.h"

{{#each this.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}ptu_timer_config_t {{this.name}};
{{/each}}
#endif

