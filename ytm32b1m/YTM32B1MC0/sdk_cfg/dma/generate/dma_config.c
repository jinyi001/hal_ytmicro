{{ytCopyright "dma_config.c"}}

#include <stddef.h>
#include "dma_config.h"


{{#each this.data.chs}}
/*{{this.name}}*/
{{#isTruthy this.callback}}extern void {{{this.callback}}}(void *parameter, dma_chn_status_t status);{{/isTruthy}}
{{#isTruthy this.callbackParam}}extern void* {{{this.callbackParam}}};{{/isTruthy}}
{{#isTruthy this.readonly}}const {{/isTruthy}}dma_channel_config_t {{this.name}} = {
    .virtChnConfig={{this.virtChnConfig}},
    .source={{this.source}},
    .callback={{{this.callback}}},
    .callbackParam={{{this.callbackParam}}},
};
{{/each}}


const dma_channel_config_t *const dmaChnConfigArray[NUM_OF_CONFIGURED_DMA_CHANNEL] = {
{{#each this.data.chs}}
    &{{this.name}},
{{/each}}
};

const dma_user_config_t dmaController_InitConfig = {
    .haltOnError = {{this.data.userConfig.haltOnError}},
{{#if (getGlobalVar 'meta.LINFlexD_DMA_Errata')}}
    .maxChannelForChLink={{this.data.userConfig.errataDma}},
{{/if}}
};

{{#each this.data.chs}}
dma_chn_state_t {{this.name}}_State;
{{/each}}



dma_chn_state_t *const dmaChnState[NUM_OF_CONFIGURED_DMA_CHANNEL]={
{{#each this.data.chs}}
    &{{this.name}}_State,
{{/each}}
};

dma_state_t dmaState;

{{#if this.data.transEnable}}
{{#each this.data.trans}}
dma_loop_transfer_config_t {{this.name}}LoopConfig = {
    .triggerLoopIterationCount={{this.loopTransferConfig.triggerLoopIterationCount}},
    .srcOffsetEnable={{this.loopTransferConfig.srcOffsetEnable}},
    .dstOffsetEnable={{this.loopTransferConfig.dstOffsetEnable}},
    .triggerLoopOffset={{this.loopTransferConfig.triggerLoopOffset}},
    .transferLoopChnLinkEnable={{this.loopTransferConfig.transferLoopChnLinkEnable}},
    .transferLoopChnLinkNumber={{this.loopTransferConfig.transferLoopChnLinkNumber}},
    .triggerLoopChnLinkEnable={{this.loopTransferConfig.triggerLoopChnLinkEnable}},
    .triggerLoopChnLinkNumber={{this.loopTransferConfig.triggerLoopChnLinkNumber}},
};

dma_transfer_config_t {{this.name}} = {
    .srcAddr=(uint32_t){{this.srcAddr}},
    .destAddr=(uint32_t){{this.destAddr}},
    .srcOffset={{this.srcOffset}},
    .destOffset={{this.destOffset}},
    .srcTransferSize={{this.srcTransferSize}},
    .destTransferSize={{this.destTransferSize}},
    .srcModulo={{this.srcModulo}},
    .destModulo={{this.destModulo}},
    .transferLoopByteCount={{this.transferLoopByteCount}},
    .srcLastAddrAdjust={{this.srcLastAddrAdjust}},
    .destLastAddrAdjust={{this.destLastAddrAdjust}},
    .interruptEnable={{this.interruptEnable}},
    .loopTransferConfig=&{{this.name}}LoopConfig,
};
{{/each}}
{{/if}}

