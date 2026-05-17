{{ytCopyright "rtc_config.h"}}



#ifndef __RTC_CONFIG_H__
#define __RTC_CONFIG_H__




#include "rtc_driver.h"


{{#each this.data}}
/*{{this.name}}*/
extern {{#isTruthy this.readonly}}const {{/isTruthy}}rtc_init_config_t {{this.name}};
{{/each}}



#endif

