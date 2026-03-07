{{ytCopyright "adc_config.h"}}



#ifndef __ADC_CONFIG_H__
#define __ADC_CONFIG_H__

#include "adc_driver.h"

{{#each this.data}}
/* {{this.name}} */
extern {{#isTruthy this.readonly}}const {{/isTruthy}}adc_converter_config_t {{this.name}};
{{/each}}

#endif


