{{ytCopyright "uart_config.c"}}


#include "uart_config.h"


{{#each this.data}}
/*{{this.name}}*/
uart_state_t {{this.name}}_State;
{{#isTruthy this.readonly}}const {{/isTruthy}}uart_user_config_t {{this.name}} = {
    .baudRate={{this.baudrate}}U,
    .parityMode={{this.parityType}},
    .stopBitCount={{this.stopBitsCount}},
    .bitCountPerChar={{this.wordLength}},
{{#if (getGlobalVar 'meta.dma_num')}}
    .transferType={{this.transferType}},
    .rxDMAChannel={{this.rxDMAChannel}},
    .txDMAChannel={{this.txDMAChannel}},
{{else}}
    .transferType=UART_USING_INTERRUPTS,
{{/if}}
    .idleErrorIntEnable={{isTruthy this.idleErrorIntEnable}},
};
{{/each}}

