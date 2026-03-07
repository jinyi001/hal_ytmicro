{{ytCopyright acmp_config.h}}



#ifndef __ACMP_CONFIG_H__
#define __ACMP_CONFIG_H__

#include "acmp_driver.h"

{{#each this.data}}
/* {{this.name}} */
extern {{#isTruthy this.readonly}}const {{/isTruthy}}acmp_config_t {{this.name}};
{{/each}}

#endif


