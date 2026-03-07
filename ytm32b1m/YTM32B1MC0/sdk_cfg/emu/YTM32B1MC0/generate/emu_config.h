{{ytCopyright "emu_config.h"}}



#ifndef __EMU_CONFIG_H__
#define __EMU_CONFIG_H__




#include "emu_driver.h"

{{#each this.data}}
extern {{#isTruthy this.readonly}}const {{/isTruthy}}emu_config_t {{this.name}};
{{/each}}

#endif

