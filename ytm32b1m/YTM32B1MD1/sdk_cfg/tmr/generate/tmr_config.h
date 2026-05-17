{{ytCopyright "tmr_config.h"}}



#ifndef __TMR_CONFIG_H__
#define __TMR_CONFIG_H__




#include "tmr_driver.h"

{{#each this.data.channel}}
extern const tmr_channel_config_t tmr_channel_{{this.channel}};
{{/each}}
extern {{#isTruthy this.data.readonly}}const {{/isTruthy}}tmr_config_t {{this.data.name}};

#endif

