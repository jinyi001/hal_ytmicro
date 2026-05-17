{{ytCopyright "linflexd_uart_config.h"}}



#ifndef __LINFLEXD_UART_CONFIG_H__
#define __LINFLEXD_UART_CONFIG_H__




#include "linflexd_uart_driver.h"



{{#each this.data}}
/*{{this.name}}*/
extern linflexd_uart_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}}linflexd_uart_user_config_t {{this.name}};
{{/each}}





#endif

