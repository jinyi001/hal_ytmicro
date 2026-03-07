{{ytCopyright "pin_mux.h"}}


#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

#include "pins_driver.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*! @brief Definitions for BOARD_InitPins Functional Group */
{{#each this.data.cfgs}}
/*! @brief User number of pins cfg {{this.name}} */
#define NUM_OF_CONFIGURED_PINS{{@index}}                        ({{this.pins.length}}U)
{{/each}}

{{#each this.data.cfgs}}
{{#if (gt this.pins.length 0)}}
/*! @brief User configuration cfg {{this.name}} array */
extern const pin_settings_config_t g_pin_mux_{{this.name}}[NUM_OF_CONFIGURED_PINS{{@index}}];
{{/if}}
{{/each}}

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/

