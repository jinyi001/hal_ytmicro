{{ytCopyright "crc_config.h"}}



#ifndef __CRC_CONFIG_H__
#define __CRC_CONFIG_H__




#include "crc_driver.h"



{{#each this.data}}
/*{{this.name}}*/
extern {{#isTruthy this.readonly}}const {{/isTruthy}}crc_user_config_t {{this.name}};
{{/each}}





#endif

