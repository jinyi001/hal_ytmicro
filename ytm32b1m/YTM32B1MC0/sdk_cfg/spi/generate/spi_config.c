{{ytCopyright "spi_config.c"}}

#include <stddef.h>
#include "spi_config.h"

/* SPI master */
{{#each this.data.master}}
/*{{this.name}}*/
{{#isTruthy this.callback}}extern void {{{this.callback}}}(void *driverState, spi_event_t event, void *userData);{{/isTruthy}}
{{#isTruthy this.callbackParam}}extern void* {{{this.callbackParam}}};{{/isTruthy}}

spi_state_t {{this.name}}_State;
{{#isTruthy this.readonly}}const {{/isTruthy}}spi_master_config_t {{this.name}} = {
    .bitsPerSec={{this.baudrate}}U,
    .whichPcs={{this.pcs}},
    .pcsPolarity={{this.polarity}},
    .isPcsContinuous={{this.isPcsContinuous}},
    .bitcount={{this.bitcount}},
    .clkPhase={{this.clkPhase}},
    .clkPolarity={{this.clkPolarity}},
    .lsbFirst={{this.lsbFirst}},
    .transferType={{this.transferType}},
    .rxDMAChannel={{this.rxDMAChannel}},
    .txDMAChannel={{this.txDMAChannel}},
    .callback={{{this.callback}}},
    .callbackParam={{{this.callbackParam}}},
    .width={{this.width}},
};
{{/each}}
/* SPI slave */
{{#each this.data.slave}}
/*{{this.name}}*/
{{#isTruthy this.callback}}extern void {{{this.callback}}}(void *driverState, spi_event_t event, void *userData);{{/isTruthy}}
{{#isTruthy this.callbackParam}}extern void* {{{this.callbackParam}}};{{/isTruthy}}

spi_state_t {{this.name}}_State;
spi_slave_config_t {{this.name}} = {
    .whichPcs={{this.pcs}},
    .pcsPolarity={{this.polarity}},
    .bitcount={{this.bitcount}},
    .clkPhase={{this.clkPhase}},
    .clkPolarity={{this.clkPolarity}},
    .lsbFirst={{this.lsbFirst}},
    .transferType={{this.transferType}},
    .rxDMAChannel={{this.rxDMAChannel}},
    .txDMAChannel={{this.txDMAChannel}},
    .callback={{{this.callback}}},
    .callbackParam={{{this.callbackParam}}},
    .width={{this.width}},
};
{{/each}}

