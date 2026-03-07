{{ytCopyright "ltmr_config.h"}}



#ifndef __LPTMR_CONFIG_H__
#define __LPTMR_CONFIG_H__




#include "lptmr_driver.h"

extern {{#isTruthy this.data.readonly}}const {{/isTruthy}}lptmr_config_t {{this.data.name}};

#endif

