{{ytCopyright "adc_config.c"}}

#include "adc_config.h"

{{#each this.data}}

{{#if (gt (getGlobalVar 'meta.adc_wdg_inst') 1)}}
{{#if (gt this.adcCompareCnt 0)}}
{{#isTruthy this.readonly}}const {{/isTruthy}}adc_compare_config_t {{this.name}}CompCfg[{{this.adcCompareCnt}}] = {
{{#each (first this.compareConfig this.adcCompareCnt)}}
    {
        .comWdchIndex={{#isTruthy this.comWdchIndex}}{{this.comWdchIndex}}{{else}}0{{/isTruthy}},    
        .compareEnable={{this.compareEnable}},
        .compareAllChannelEnable={{this.compareAllChannelEnable}},
{{#isTruthy this.compChalSel}}        .compChalSel={{this.compChalSel}},{{/isTruthy}}
        .compHigh={{this.compHigh}},
        .compLow={{this.compLow}},
        .compIntEnable={{this.compIntEnable}},  
        .effectiveMode={{this.effectiveMode}},
    },
{{/each}}
};
{{/if}}
{{/if}}

/* {{this.name}} */
{{#isTruthy this.readonly}}const {{/isTruthy}}adc_converter_config_t {{this.name}}={
    .clockDivider={{this.clockDivider}},
    .startTime={{this.startTime}},
    .sampleTime={{this.sampleTime}},
    .overrunMode={{this.overrunMode}},
    .autoOffEnable={{this.autoOffEnable}},
    .waitEnable={{this.waitEnable}},
    .trigger={{this.trigger}},
    .align={{this.align}},
    .resolution={{this.resolution}},
    .dmaWaterMark={{this.dmaWaterMark}},
    .dmaEnable={{this.dmaEnable}},
    .sequenceConfig={
        .sequenceMode={{this.sequenceConfig.sequenceMode}},
        .sequenceIntEnable={{this.sequenceConfig.sequenceIntEnable}},
        .convIntEnable={{this.sequenceConfig.convIntEnable}},
        .readyIntEnable={{this.sequenceConfig.readyIntEnable}},
        .ovrunIntEnable={{this.sequenceConfig.ovrunIntEnable}},
        .sampIntEnable={{this.sequenceConfig.sampIntEnable}},
        .channels={
            {{#each this.sequenceConfig.channels}}
            {{this.channel}},
            {{/each}}
        },
        .totalChannels={{this.sequenceConfig.channels.length}},
    },
{{#if (gt (getGlobalVar 'meta.adc_wdg_inst') 1)}}
    .adcCompareCnt={{this.adcCompareCnt}},
    {{#if (gt this.adcCompareCnt 0)}}
    .compareConfig=(adc_compare_config_t*){{this.name}}CompCfg, 
    {{/if}}
{{else}}
    .compareConfig={
{{#each this.compareConfig}}
{{#if (lt @index 1)}}
        .compareEnable={{this.compareEnable}},
        .compareAllChannelEnable={{this.compareAllChannelEnable}},
{{#isTruthy this.compChalSel}}        .compChalSel={{this.compChalSel}},{{/isTruthy}}
        .compHigh={{this.compHigh}},
        .compLow={{this.compLow}},
        .compIntEnable={{this.compIntEnable}},
    },
{{/if}}
{{/each}}
{{/if}}
};
{{/each}}


