{{ytCopyright "acmp_config.c"}}

#include "acmp_config.h"

{{#each this.data}}
/* comparatorConfig*/
static acmp_comparator_config_t {{this.name}}ComparatorConfig={
{{#if (getGlobalVar 'meta.FEATURE_ACMP_HAS_CLK_SRC_SEL')}}
    .acmpClkSrc={{this.comparator.acmpClkSrc}},
{{/if}}
    .sampleMode={{this.comparator.sampleMode}},
    .edgeSelection={{this.comparator.edgeSelection}},
{{#if (getGlobalVar 'meta.FEATURE_ACMP_HAS_OFFSET_CONTROL')}}
    .hardBlockOffset={{this.comparator.hardBlockOffset}},
{{/if}}
    .hysteresisLevel={{this.comparator.hysteresisLevel}},
    .outputSelect={{this.comparator.outputSelect}},
    .outputPolarity={{this.comparator.outputPolarity}},
    .powerMode={{this.comparator.powerMode}},
    .filterEnable={{this.comparator.filterEnable}},
    .filterClkSrc={{this.comparator.filterClkSrc}},
    .filterSamplePeriod={{this.comparator.filterSamplePeriod}},
    .filterSampleCount={{this.comparator.filterSampleCount}},
{{#if (getGlobalVar 'meta.FEATURE_ACMP_HAS_AUTODIS')}}
    .autoDisableHardBlock={{this.comparator.autoDisableHardBlock}},
{{/if}}
    .interruptEnable={{this.comparator.interruptEnable}},
    .dmaTriggerEnable={{this.comparator.dmaTriggerEnable}},
};

/* dacConfig */
static acmp_dac_config_t {{this.name}}DacConfig={
{{#if (getGlobalVar 'meta.FEATURE_ACMP_HAS_DAC_VOLTAGE_REF_SRC')}}
    .voltageReferenceSource={{this.dac.voltageReferenceSource}},
{{/if}}
    .enable={{this.dac.enable}},
{{#if (getGlobalVar 'meta.FEATURE_ACMP_HAS_DAC_OUTPUT')}}
    .outputEnable={{this.dac.outputEnable}},
{{/if}}
    .voltage={{this.dac.voltage}},
};

/* muxConfig */
static acmp_mux_config_t {{this.name}}MuxConfig={
    .positiveInputSrc={{this.mux.positiveInputSrc}},
    .negativeInputSrc={{this.mux.negativeInputSrc}},
    .positiveInputChnSel={{this.mux.positiveInputChnSel}},
    .negativeInputChnSel={{this.mux.negativeInputChnSel}},
};

/* continuousConfig */
static acmp_continuous_config_t {{this.name}}ContinuousConfig={
    .continuousEnable={{this.continuous.continuousEnable}},
    .continuousMode={{this.continuous.continuousMode}},
{{#if (getGlobalVar 'meta.FEATURE_ACMP_HAS_TRIG_MODE_GATE')}}
    .oneshotTriggerEnable={{this.continuous.oneshotTriggerEnable}},
{{/if}}
    .fixedPort={{this.continuous.fixedPort}},
    .samplePeriod={{this.continuous.samplePeriod}},
    .samplePosition={{this.continuous.samplePosition}},
    .continuousInterruptEnable={{this.continuous.continuousInterruptEnable}},
    .channelConfig={
{{#each this.continuous.channelConfig}}
        {
            .enable={{this.enable}},
            .expectation={{this.expectation}},
        },
{{/each}}
    },
    
};

/* {{this.name}} */
{{#isTruthy this.readonly}}const {{/isTruthy}}acmp_config_t {{this.name}}={
    .comparatorConfig=&{{this.name}}ComparatorConfig,
    .muxConfig=&{{this.name}}MuxConfig,
    .dacConfig=&{{this.name}}DacConfig,
    .continuousConfig=&{{this.name}}ContinuousConfig,
};
{{/each}}


