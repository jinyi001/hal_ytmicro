{{ytCopyright "linflexd_uart_config.c"}}


#include "linflexd_uart_config.h"


{{#each this.data}}
/*{{this.name}}*/
linflexd_uart_state_t {{this.name}}_State;
{{#isTruthy this.readonly}}const {{/isTruthy}}linflexd_uart_user_config_t {{this.name}} = {
    .baudRate={{this.baudrate}}U,
    .parityCheck={{this.parityCheck}},
    .parityType={{this.parityType}},
    .stopBitsCount={{this.stopBitsCount}},
    .wordLength={{this.wordLength}},
    .txTransferType={{this.txTransferType}},
    .rxTransferType={{this.rxTransferType}},
    .txDMAChannel={{this.txDMAChannel}},
    .rxDMAChannel={{this.rxDMAChannel}},
};
{{/each}}

