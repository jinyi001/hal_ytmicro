{{ytCopyright "fee_config.c"}}

#include <stddef.h>
#include "fee_config.h"


/* general */
static {{#isTruthy this.data.flashConfig.readonly}}const {{/isTruthy}}Fls_SectorType sectorList[{{this.data.flashConfig.sectorList.length}}]={
{{#each this.data.flashConfig.sectorList}}
    {
        .sectorId={{this.sectorId}},
        .sectorStartAddress={{this.sectorStartAddress}},
        .sectorSize={{this.sectorSize}},
        .pageSize={{this.pageSize}},
        .sectorHwStartAddress={{this.sectorHwStartAddress}},
        .asyncAccess={{#if this.asyncAccess}}1{{else}}0{{/if}},
    },
{{/each}}
};

{{#isTruthy this.data.flashConfig.jobEndNotificationFunPtr}}extern void {{{this.data.flashConfig.jobEndNotificationFunPtr}}}(void);{{/isTruthy}}
{{#isTruthy this.data.flashConfig.jobErrorNotificationFunPtr}}extern void {{{this.data.flashConfig.jobErrorNotificationFunPtr}}}(void);{{/isTruthy}}
{{#isTruthy this.data.flashConfig.acEraseFunPtr}}extern void {{{this.data.flashConfig.acEraseFunPtr}}}(const EFM_Type* base, Fls_CallbackType callBack);{{/isTruthy}}
{{#isTruthy this.data.flashConfig.acWriteFunPtr}}extern void {{{this.data.flashConfig.acWriteFunPtr}}}(const EFM_Type* base, Fls_CallbackType callBack);{{/isTruthy}}


static {{#isTruthy this.data.flashConfig.readonly}}const {{/isTruthy}}Fls_ConfigType flashConfig={
    .acEraseFunPtr={{{this.data.flashConfig.acEraseFunPtr}}},
    .acWriteFunPtr={{{this.data.flashConfig.acWriteFunPtr}}},
    .jobEndNotificationFunPtr={{{this.data.flashConfig.jobEndNotificationFunPtr}}},
    .jobErrorNotificationFunPtr={{{this.data.flashConfig.jobErrorNotificationFunPtr}}},
    .eDefaultMode={{this.data.flashConfig.eDefaultMode}},
    .maxReadFastMode={{this.data.flashConfig.maxReadFastMode}},
    .maxReadNormalMode={{this.data.flashConfig.maxReadNormalMode}},
    .maxWriteFastMode={{this.data.flashConfig.maxWriteFastMode}},
    .maxWriteNormalMode={{this.data.flashConfig.maxWriteNormalMode}},
    .ConfigSectorNum={{this.data.flashConfig.sectorList.length}},
    .sectorList=sectorList,
};


/* cluster */
{{#each this.data.clusterConfig}}
static {{#isTruthy this.data.flashConfig.readonly}}const {{/isTruthy}}Fee_ClusterType clrType{{@index}}[{{this.clusterList.length}}]={
{{#each this.clusterList}}
    {
        .StartAddr={{this.StartAddr}},
        .Length={{this.Length}},
    },
{{/each}}
};
{{/each}}

static {{#isTruthy this.data.flashConfig.readonly}}const {{/isTruthy}}Fee_ClusterGroupType clusterConfig[{{this.data.clusterConfig.length}}]={
{{#each this.data.clusterConfig}}
    {
        .ReservedSize={{this.ReservedSize}},
        .ClrCount={{this.clusterList.length}},
        .ClrPtr=clrType{{@index}},
    },
{{/each}}
};

/* block */
static {{#isTruthy this.data.flashConfig.readonly}}const {{/isTruthy}}Fee_BlockConfigType blockConfig[{{this.data.blockConfig.length}}]={
{{#each (sortBy this.data.blockConfig 'BlockNumber')}}
    {
        .BlockNumber={{this.BlockNumber}},
        .BlockSize={{this.BlockSize}},
        .ClrGrp={{this.ClrGrp}},
        .ImmediateData={{#if this.ImmediateData}}1{{else}}0{{/if}},
        .BlockAssignment={{this.BlockAssignment}},
    },
{{/each}}
};



/* top */

{{#isTruthy this.data.flashConfig.readonly}}const {{/isTruthy}}Fee_ModuleUserConfig_t {{this.data.flashConfig.name}}={
    .blockCnt={{this.data.blockConfig.length}},
    .clusterCnt={{this.data.clusterConfig.length}},
    .clusterConfigPtr=clusterConfig,
    .blockConfigPtr=blockConfig,
    .flashConfigPtr=&flashConfig,
};





