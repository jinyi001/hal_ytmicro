{{ytCopyright "linflexd_lin_config.h"}}



#ifndef __LINFLEXD_LIN_CONFIG_H__
#define __LINFLEXD_LIN_CONFIG_H__




#include "linflexd_lin_driver.h"



{{#each this.data}}
/*{{this.name}}*/
extern linflexd_state_t {{this.name}}_State;
extern linflexd_user_config_t {{this.name}};
{{/each}}





#endif

