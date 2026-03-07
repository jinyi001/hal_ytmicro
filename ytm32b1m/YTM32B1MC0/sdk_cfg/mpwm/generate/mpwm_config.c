{{ytCopyright "mpwm_config.c"}}

#include <stddef.h>
#include "mpwm_config.h"

/*
 * Common state
 */
{{#each this.data.common}}
mpwm_state_t {{this.state_name}};

{{#if (getGlobalVar 'meta.FEATURE_MPWM_HAS_IPC_CLOCK_SOURCE')}}
const mpwm_user_config_t {{this.name}} = {
    .clkSrc = {{this.clock_source}},
};
{{/if}}
{{/each}}

/*
 * Common timer
 */
{{#if this.data.ct.enable}}
{{#each this.data.ct.data}}

mpwm_ct_ch_config_t {{this.name}}ChConfig[{{this.channelNum}}] = {
{{#each (first this.timerChannelConfig this.channelNum)}}
    {
        .hwChannelId = {{this.hwChannelId}},
        .finalValue = {{this.finalValue}},
        .clkPrs = {{this.clkPrs}},
        .reloadSrc = {{this.reloadSrc}},
        .hwTrigEn = {{this.hwTrigEn}},
        .debugMode = {{this.debugMode}},
        .countMode = {{this.countMode}},
        .timerOverflowIntEn = {{this.timerOverflowIntEn}},
    }
{{/each}}
};

{{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_ct_config_t {{this.name}}={
    .channelNum={{this.channelNum}},
    .timerChannelConfig={{this.name}}ChConfig,
};
{{/each}}
{{/if}}


/*
 * PWM
 */
{{#if this.data.pwm.enable}}
{{#each this.data.pwm.data}}

mpwm_pwm_ch_config_t {{this.name}}ChConfig[{{this.channelNum}}]={
{{#each (first this.pwmChannelConfig this.channelNum)}}
    {
        .hwChannelId = {{this.hwChannelId}},
        .clkPrs = {{this.clkPrs}},
        .frequencyHz = {{this.frequencyHz}},
        .unit = {{this.unit}},
        .dutyCyclePercent = {{this.dutyCyclePercent}},
        .alignMode = {{this.alignMode}},
        .outputInit = {{this.outputInit}},
        .debugMode = {{this.debugMode}},
        .reloadSrc = {{this.reloadSrc}},
        .outputTrigConfig = {
            {{this.outputTrigConfig.cmpTrigOutputEn}},
            {{this.outputTrigConfig.perTrigOutputEn}},
        },
        .intEn = {{this.intEn}},
        .timerOverflowIntEn = {{this.timerOverflowIntEn}},
    },
{{/each}}
};

{{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_pwm_config_t {{this.name}}={
    .channelNum={{this.channelNum}},
    .pwmChannelConfig={{this.name}}ChConfig,
};
{{/each}}
{{/if}}


/*
 * Output compare
 */
{{#if this.data.oc.enable}}
{{#each this.data.oc.data}}

mpwm_oc_ch_config_t {{this.name}}ChConfig[{{this.channelNum}}]={
{{#each (first this.outputCompareChannelConfig this.channelNum)}}
    {
        .hwChannelId = {{this.hwChannelId}},
        .clkPrs = {{this.clkPrs}},
        .maxCountValue = {{this.maxCountValue}},
        .compareValue = {{this.compareValue}},
        .compareOutput = {{this.compareOutput}},
        .outputInit = {{this.outputInit}},
        .debugMode = {{this.debugMode}},
        .reloadSrc = {{this.reloadSrc}},
        .outputTrigConfig = {
            {{this.outputTrigConfig.cmpTrigOutputEn}},
            {{this.outputTrigConfig.perTrigOutputEn}},
        },
        .intEn = {{this.intEn}},
        .timerOverflowIntEn = {{this.timerOverflowIntEn}},
    },
{{/each}}
};

{{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_oc_config_t {{this.name}}={
    .channelNum={{this.channelNum}},
    .outputCompareChannelConfig={{this.name}}ChConfig,
};
{{/each}}
{{/if}}


/*
 * Input capture
 */
{{#if this.data.ic.enable}}
{{#each this.data.ic.data}}

mpwm_ic_ch_config_t {{this.name}}ChConfig[{{this.channelNum}}]={
{{#each (first this.inputCaptureChannelConfig this.channelNum)}}
    {
        .hwChannelId = {{this.hwChannelId}},
        .clkPrs = {{this.clkPrs}},
        .cntAutoClr = {{this.cntAutoClr}},
        .overwrite = {{this.overwrite}},
        .edge = {{this.edge}},
        .dmaEn = {{this.dmaEn}},
        .countMode = {{this.countMode}},
        .maxCountValue = {{this.maxCountValue}},
        .debugMode = {{this.debugMode}},
        .intEn = {{this.intEn}},
        .timerOverflowIntEn = {{this.timerOverflowIntEn}},
    },
{{/each}}
};

{{#if (getGlobalVar 'meta.FEATURE_MPWM_HAS_COMBINATION_CAPTURE')}}
{{#if this.channelPairNum}}
mpwm_ic_ch_pair_config_t {{this.name}}ChPairConfig[{{this.channelPairNum}}]={
{{#each (first this.inputCaptureChannelPairConfig this.channelPairNum)}}
    {
        .hwChannelPairId = {{this.hwChannelPairId}},
        .combEn = {{this.combEn}},
        .combSrc = {{this.combSrc}},
    },
{{/each}}
};
{{/if}}
{{/if}}

{{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_ic_config_t {{this.name}}={
    .channelNum = {{this.channelNum}},
    .inputCaptureChannelConfig = {{this.name}}ChConfig,
{{#if (getGlobalVar 'meta.FEATURE_MPWM_HAS_COMBINATION_CAPTURE')}}
{{#if this.channelPairNum}}
    .channelPairNum = {{this.channelPairNum}},
    .inputCaptureChannelPairConfig = {{this.name}}ChPairConfig,
{{/if}}
{{/if}}
};
{{/each}}
{{/if}}


/*
 * Pulse count
 */
{{#if this.data.pc.enable}}
{{#each this.data.pc.data}}
mpwm_pc_ch_config_t {{this.name}}ChConfig[{{this.channelNum}}]={
{{#each (first this.pulseCounterChannelConfig this.channelNum)}}
    {
        .hwChannelId = {{this.hwChannelId}},
        .clkPrs = {{this.clkPrs}},
        .edge = {{this.edge}},
        .maxPulseNum = {{this.maxPulseNum}},
        .cmpPulseNum = {{this.cmpPulseNum}},
        .dmaEn = {{this.dmaEn}},
        .countMode = {{this.countMode}},
        .debugMode = {{this.debugMode}},
        .reloadSrc = {{this.reloadSrc}},
        .intEn = {{this.intEn}},
        .timerOverflowIntEn = {{this.timerOverflowIntEn}},
    },
{{/each}}
};

{{#isTruthy this.readonly}}const {{/isTruthy}}mpwm_pc_config_t {{this.name}}={
    .channelNum={{this.channelNum}},
    .pulseCounterChannelConfig={{this.name}}ChConfig,
};
{{/each}}
{{/if}}


