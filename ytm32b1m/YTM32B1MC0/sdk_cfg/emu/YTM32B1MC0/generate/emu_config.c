{{ytCopyright "emu_config.c"}}

#include "emu_config.h"

{{#each this.data}}
{{#isTruthy this.readonly}}const {{/isTruthy}}emu_config_t {{this.name}}=
{
    .channel={{this.channel}},
    .injectErrAddr={{this.injectErrAddr}},
    .injectErrSetInfo={
        .injectBitsType={{this.injectErrSetInfo.injectBitsType}},
        .dataBit={{this.injectErrSetInfo.dataBit}},
        .chkBit={{this.injectErrSetInfo.chkBit}},
    },
    .interruptEnable={{this.interruptEnable}},
};
{{/each}}

