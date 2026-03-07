{{ytCopyright "ptmr_config.h"}}



#ifndef __PTMR_CONFIG_H__
#define __PTMR_CONFIG_H__




#include "ptmr_driver.h"

{{#each this.data.channel}}
extern const ptmr_user_channel_config_t ptmr_channel_{{this.channel}};
{{/each}}
extern {{#isTruthy this.data.readonly}}const {{/isTruthy}}ptmr_user_config_t {{this.data.name}};

#endif

