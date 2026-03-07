{{ytCopyright "spi_config.h"}}



#ifndef __SPI_CONFIG_H__
#define __SPI_CONFIG_H__




#include "spi_shared_function.h"
#include "spi_master_driver.h"
#include "spi_slave_driver.h"

/* SPI master */
{{#each this.data.master}}
/*{{this.name}}*/
extern spi_state_t {{this.name}}_State;
extern {{#isTruthy this.readonly}}const {{/isTruthy}}spi_master_config_t {{this.name}};
{{/each}}
/* SPI slave */
{{#each this.data.slave}}
/*{{this.name}}*/
extern spi_state_t {{this.name}}_State;
extern spi_slave_config_t {{this.name}};
{{/each}}




#endif

