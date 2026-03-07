{{ytCopyright "fee_config.h"}}



#ifndef __FEE_CONFIG_H__
#define __FEE_CONFIG_H__



#include "fee.h"

#define FLS_CONFIGURED_SECTOR_NUMBER {{this.data.flashConfig.sectorList.length}}U/*Count of user configuration flash sectors*/
#define FEE_NUMBER_OF_CLUSTER_GROUPS {{this.data.clusterConfig.length}}U  /* Number of configured Fee cluster group */
#define FEE_CRT_CFG_NR_OF_BLOCKS  {{this.data.blockConfig.length}}U /* Number of configured Fee blocks */


/* Symbolic names of configured Fee blocks */
{{#each (sortBy this.data.blockConfig 'BlockNumber')}}
#define {{this.BlockName}}      ({{this.BlockNumber}}U)
{{/each}}




extern {{#isTruthy this.data.flashConfig.readonly}}const {{/isTruthy}}Fee_ModuleUserConfig_t {{this.data.flashConfig.name}};

#endif


