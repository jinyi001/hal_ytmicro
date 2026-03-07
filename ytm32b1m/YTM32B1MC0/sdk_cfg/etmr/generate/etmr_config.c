{{ytCopyright "etmr_config.c"}}

#include <stddef.h>
#include "etmr_config.h"

/*
 * Common
*/
{{#each this.data.common}}

etmr_pwm_sync_t {{this.name}}PwmSync={
    .regSyncFreq={{this.syncMethod.regSyncFreq}},
    .regSyncSel={{this.syncMethod.regSyncSel}},
    .cntInitSyncSel={{this.syncMethod.cntInitSyncSel}},
    .maskOutputSyncSel={{this.syncMethod.maskOutputSyncSel}},
    .regSyncTrigSrc={{this.syncMethod.regSyncTrigSrc}},
    .cntInitSyncTrigSrc={{this.syncMethod.cntInitSyncTrigSrc}},
    .maskOutputSyncTrigSrc={{this.syncMethod.maskOutputSyncTrigSrc}},
    .hwTrigFromTmuEnable={{this.syncMethod.hwTrigRegLoadEnable}},
    .hwTrigFromCimEnable={{this.syncMethod.hwTrigCounterLoadEnable}},
    .hwTrigFromPadEnable={{this.syncMethod.hwTrigMaskLoadEnable}},
};

{{#isTruthy this.outputTrigConfig.channelTrigParamConfig.length}}
etmr_trig_ch_param_t {{this.name}}TrigConfChannels[{{this.outputTrigConfig.channelTrigParamConfig.length}}]={
{{#each this.outputTrigConfig.channelTrigParamConfig}}
    {
        .channelId={{this.channelId}},
        .channelVal0MatchTrigEn={{this.channelVal0MatchTrigEn}},
        .channelVal1MatchTrigEn={{this.channelVal1MatchTrigEn}},
    },
{{/each}}
};
{{/isTruthy}}

etmr_trig_config_t {{this.name}}TrigConf={
    .trigSrc={{this.outputTrigConfig.trigSrc}},
    .pwmOutputChannel={{this.outputTrigConfig.pwmOutputChannel}},
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_OUTPUT_TRIGGER_WIDTH')}}
    .outputTrigWidth={{this.outputTrigConfig.outputTrigWidth}},
{{/if}}
    .outputTrigFreq={{this.outputTrigConfig.outputTrigFreq}},
    .modMatchTrigEnable={{this.outputTrigConfig.modMatchTrigEnable}},
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_MID')}}
    .midMatchTrigEnable={{this.outputTrigConfig.midMatchTrigEnable}},
{{/if}}
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_INIT_REG')}}
    .initMatchTrigEnable={{this.outputTrigConfig.initMatchTrigEnable}},
{{/if}}
    .numOfChannels={{this.outputTrigConfig.channelTrigParamConfig.length}},
{{#isTruthy this.outputTrigConfig.channelTrigParamConfig.length}}
    .channelTrigParamConfig={{this.name}}TrigConfChannels,
{{else}}
    .channelTrigParamConfig=NULL,
{{/isTruthy}}
};

{{#isTruthy this.readonly}}const {{/isTruthy}}etmr_user_config_t {{this.name}}={
    .etmrClockSource={{this.etmrClockSource}},
    .etmrPrescaler={{this.etmrPrescaler}},
    .debugMode={{this.debugMode}},
    .syncMethod=&{{this.name}}PwmSync,
    .outputTrigConfig=&{{this.name}}TrigConf,
    .isTofIntEnabled={{this.isTofIntEnabled}},
};

etmr_state_t {{this.name}}_State;

{{/each}}

/*
 * MC
*/
{{#if this.data.mc.enable}}
{{#each this.data.mc.data}}

{{#isTruthy this.readonly}}const {{/isTruthy}}etmr_timer_param_t {{this.name}}={
    .initialValue={{this.initialValue}},
    .finalValue={{this.finalValue}},
};
{{/each}}
{{/if}}


/*
 * PWM
*/
{{#if this.data.pwm.enable}}
{{#each this.data.pwm.data}}

etmr_pwm_ch_param_t {{this.name}}IndChConfig[{{this.nNumIndependentPwmChannels}}]={
{{#each (first this.pwmIndependentChannelConfig this.nNumIndependentPwmChannels)}}
    {
        .hwChannelId={{this.hwChannelId}},
        .polarity={{this.polarity}},
        .pwmSrcInvert={{this.pwmSrcInvert}},
        .align={{this.align}},
        .channelInitVal={{this.channelInitVal}},
        .typeOfUpdate={{this.typeOfUpdate}},
        .dutyCycle={{this.dutyCycle}},
        .offset={{this.offset}},
        .enableSecondChannelOutput={{this.enableSecondChannelOutput}},
        .secondChannelPolarity={{this.secondChannelPolarity}},
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_DOUBLE_SWITCH')}}
        .enableDoubleSwitch={{this.enableDoubleSwitch}},
{{/if}}
{{#if (getGlobalVar 'meta.FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL')}}
        .evenDeadTime={{this.evenDeadTime}},
        .oddDeadTime={{this.oddDeadTime}},
{{/if}}
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_CHANNEL_VALUE_MATCH_INTERRUPT_IN_PWM_MODE')}}
        .interruptEnable={{this.interruptEnable}},
{{/if}}
    },
{{/each}}
};

{{#if (getGlobalVar 'meta.FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL')}}
{{else}}
etmr_deadtime_param_t {{this.name}}dtConfig={
    .prs={{this.deadtimeConfig.prs}},
    .evenDeadTime={{this.deadtimeConfig.evenDeadTime}},
    .oddDeadTime={{this.deadtimeConfig.oddDeadTime}},
};
{{/if}}

etmr_fault_param_t {{this.name}}FaultConfig={
    .pwmFaultInterrupt={{this.faultConfig.pwmFaultInterrupt}},
    .faultFilterSampleCounter={{this.faultConfig.faultFilterSampleCounter}},
    .faultFilterSamplePeriod={{this.faultConfig.faultFilterSamplePeriod}},
    .faultInputStrentch={{this.faultConfig.faultInputStrentch}},
    .pwmRecoveryOpportunity={{this.faultConfig.pwmRecoveryOpportunity}},
    .pwmAutoRecoveryMode={{this.faultConfig.pwmAutoRecoveryMode}},
    .faultMode={{this.faultConfig.faultMode}},
    .etmrFaultChannelParam=
    {
        {{#each this.faultConfig.etmrFaultChannelParam}}
        {
            .faultChannelEnabled={{this.faultChannelEnabled}},
            .faultInputPolarity={{this.faultInputPolarity}},
        },
        {{/each}}
    },
    .safeState={
        {{#each this.faultConfig.safeState}}
        {{this.val}},
        {{/each}}
    }
};


{{#isTruthy this.readonly}}const {{/isTruthy}}etmr_pwm_param_t {{this.name}}={
    .nNumPwmChannels={{this.nNumIndependentPwmChannels}},
    .mode={{this.mode}},
    .uFrequencyHZ={{this.uFrequencyHZ}},
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_CNT_INIT_SRC')}}
    .counterInitValFromInitReg={{this.counterInitValFromInitReg}},
{{/if}}
    .cntVal={{#if this.cntVal}}{{this.cntVal}}{{else}}0{{/if}},
    .pwmChannelConfig={{this.name}}IndChConfig,
{{#if (getGlobalVar 'meta.FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL')}}
{{else}}
    .deadtimeConfig=&{{this.name}}dtConfig,
{{/if}}
    .faultConfig=&{{this.name}}FaultConfig,
};
{{/each}}
{{/if}}


/*
 * IC
*/
{{#if this.data.ic.enable}}
{{#each this.data.ic.data}}

{{#each (first this.inputChConfig this.numChannels)}}

{{#isTruthy this.channelsCallbacks}}extern void {{{this.channelsCallbacks}}}(ic_event_t event, void *userData);{{/isTruthy}}
{{#isTruthy this.channelsCallbacksParams}}extern void* {{{this.channelsCallbacksParams}}};{{/isTruthy}}

{{/each}}

etmr_ic_ch_param_t {{this.name}}InputCh[{{this.numChannels}}]={
{{#each (first this.inputChConfig this.numChannels)}}
    {
        .hwChannelId={{this.hwChannelId}},
        .edge={{this.edge}},
        .measurementType={{this.measurementType}},
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_CAPTURE_HOLD')}}
        .capHoldEn={{this.capHoldEn}},  
{{/if}}
        .filterSampleCounter={{this.filterSampleCounter}},
        .filterSamplePeriod={{this.filterSamplePeriod}},
        .interruptEnable={{this.interruptEnable}},
        .dmaEnable={{this.dmaEnable}},
        .enableNotification={{this.enableNotification}},
        .channelsCallbacks={{{this.channelsCallbacks}}},
        .channelsCallbacksParams={{#isTruthy this.channelsCallbacksParams}}&{{{this.channelsCallbacksParams}}}{{else}}NULL{{/isTruthy}},
    },
{{/each}}
};

{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_COMBINATION_CAPTURE')}}
{{#if this.numChannelPairs}}
etmr_ic_ch_pair_param_t {{this.name}}InputChPair[{{this.numChannelPairs}}]={
{{#each (first this.inputChPairConfig this.numChannelPairs)}}
    {
        .hwChannelPairId={{this.hwChannelPairId}},
        .combEn={{this.combEn}},
        .combSrc={{this.combSrc}},
    },
{{/each}}
};
{{/if}}
{{/if}}

{{#isTruthy this.readonly}}const {{/isTruthy}}etmr_ic_param_t {{this.name}}={
    .numChannels={{this.numChannels}},
    .countValue={{this.countValue}},
    .inputChConfig={{this.name}}InputCh,
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_COMBINATION_CAPTURE')}}
{{#if this.numChannelPairs}}
    .numChannelPairs={{this.numChannelPairs}},
    .inputChPairConfig={{this.name}}InputChPair,
{{/if}}
{{/if}}
};
{{/each}}
{{/if}}


/*
 * OC
*/
{{#if this.data.oc.enable}}
{{#each this.data.oc.data}}

etmr_oc_ch_param_t {{this.name}}OcCh[{{this.nNumOutputChannels}}]={
{{#each (first this.outputChannelConfig this.nNumOutputChannels)}}
    {
        .hwChannelId={{this.hwChannelId}},
        .channelInitVal={{this.channelInitVal}},
        .val0CmpMode={{this.val0CmpMode}},
        .val1CmpMode={{this.val1CmpMode}},
        .cmpVal0={{this.cmpVal0}},
        .cmpVal1={{this.cmpVal1}},
        .enableExternalTrigger0={{this.enableExternalTrigger0}},
        .enableExternalTrigger1={{this.enableExternalTrigger1}},
        .interruptEnable={{this.interruptEnable}},
    },
{{/each}}
};

{{#isTruthy this.readonly}}const {{/isTruthy}}etmr_oc_param_t {{this.name}}={
    .nNumOutputChannels={{this.nNumOutputChannels}},
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_CNT_INIT_SRC')}}
    .counterInitValFromInitReg={{this.counterInitValFromInitReg}},
{{/if}}
    .cntVal={{this.cntVal}},
    .maxCountValue={{this.maxCountValue}},
    .outputChannelConfig={{this.name}}OcCh,
};
{{/each}}
{{/if}}


/*
 * QD
*/
{{#if this.data.qd.enable}}
{{#each this.data.qd.data}}
{{#isTruthy this.readonly}}const {{/isTruthy}}etmr_qd_config_t {{this.name}}={
    .mode={{this.mode}},
    .qdClkPrs={{this.qdClkPrs}},
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_INIT_REG')}}
    .initVal={{this.initVal}},
{{/if}}
{{#if (getGlobalVar 'meta.FEATURE_eTMR_HAS_QUAD_DECODER_MOD')}}
    .modVal={{this.modVal}},
{{/if}}
    .qdInputFilterSampleCount={{this.qdInputFilterSampleCount}},
    .qdInputFilterSamplePeriod={{this.qdInputFilterSamplePeriod}},
    .phaseAPol={{this.phaseAPol}},
    .phaseBPol={{this.phaseBPol}},
};

etmr_qd_state_t {{this.name}}_State;
{{/each}}
{{/if}}


