{{ytCopyright "uart_config.h"}}



#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__




#include "uart_driver.h"



{{#each this.data}}
/*{{this.name}}*/
extern uart_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}}uart_user_config_t {{this.name}};
{{/each}}





#endif

