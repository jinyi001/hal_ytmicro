{{ytCopyright "i2c_config.h"}}



#ifndef __I2C_CONFIG_H__
#define __I2C_CONFIG_H__




#include "i2c_driver.h"

/* I2C master */
{{#each this.data.master}}
/*{{this.name}}*/
extern i2c_master_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}}i2c_master_user_config_t {{this.name}};
{{/each}}
/* I2C slave */
{{#each this.data.slave}}
/*{{this.name}}*/
extern i2c_slave_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}}i2c_slave_user_config_t {{this.name}};
{{/each}}




#endif

