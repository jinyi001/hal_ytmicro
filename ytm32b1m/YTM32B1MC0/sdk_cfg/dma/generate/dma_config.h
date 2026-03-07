{{ytCopyright "dma_config.h"}}



#ifndef __DMA_CONFIG_H__
#define __DMA_CONFIG_H__




#include "dma_driver.h"



#define NUM_OF_CONFIGURED_DMA_CHANNEL {{this.data.chs.length}}U


extern dma_state_t dmaState;
extern dma_chn_state_t *const dmaChnState[NUM_OF_CONFIGURED_DMA_CHANNEL];
extern const dma_user_config_t dmaController_InitConfig;
extern const dma_channel_config_t *const dmaChnConfigArray[NUM_OF_CONFIGURED_DMA_CHANNEL];
{{#if this.data.transEnable}}
{{#each this.data.trans}}
extern dma_transfer_config_t {{this.name}};
{{/each}}
{{/if}}

#endif

