{{ytCopyright "clock_config.c"}}
#include "stddef.h"
#include "clock_config.h"

/*! @brief peripheral clock {{this.name}}PeripheralClockConfig */
{{#each this.data.clockArray}}

peripheral_clock_config_t {{this.name}}PeripheralClockConfig[{{get 'length' (filter this.data.periClocks true property="userCtrl")}}] = {
{{#each (filter this.data.periClocks true property="userCtrl")}}
    {
        .clkName = {{this.name}},
        .clkGate = {{this.gate}},
        .divider = DIV_BY_{{add this.div 1}},
        .clkSrc = {{itemAt (split 'CLK_SRC_DISABLED,CLK_SRC_FIRC,CLK_SRC_SIRC,CLK_SRC_FXOSC,CLK_SRC_LPO,CLK_SRC_FAST_BUS' ',') this.periSrc}},
    },
{{/each}}
};

const scu_config_t {{this.name}}ScuConfig = {
    .fircEnable = {{isTruthy this.data.firc.enable}},
    .fircDeepSleepEnable = {{isTruthy this.data.firc.dsEnable}},
    .sircDeepSleepEnable = {{isTruthy this.data.sirc.dsEnable}},
    .sircStandbyEnable = {{isTruthy this.data.sirc.stbEnable}},
    .sysClkSrc = {{itemAt (split 'SCU_SYSTEM_CLOCK_SRC_FIRC,SCU_SYSTEM_CLOCK_SRC_FXOSC,SCU_SYSTEM_CLOCK_SRC_SIRC' ',') this.data.coreClock.ref}},
    .fxoscConfig =
        {
            .enable = {{isTruthy this.data.fxosc.enable}},
            .bypassMode = {{isTruthy this.data.fxosc.mode}},
            .gainSelection = {{this.data.fxosc.gain}},
            .frequency = {{this.data.fxosc.freq.raw}}U,
        },
    .sysDiv = SCU_SYS_CLK_DIV_BY_{{add this.data.coreClock.div 1}},
    .fastBusDiv = SCU_SYS_CLK_DIV_BY_{{add this.data.fastClock.div 1}},
    .slowBusDiv = SCU_SYS_CLK_DIV_BY_{{add this.data.slowClock.div 1}},
    .flashDiv = SCU_SYS_CLK_DIV_BY_{{add this.data.coreClock.flash 1}},
    .clockOutConfig =
        {
            .enable = {{isTruthy this.data.clkOut.enable}},
            .source = {{itemAt (split 'SCU_CLKOUT_SEL_FIRC_CLK,SCU_CLKOUT_SEL_SIRC_CLK,SCU_CLKOUT_SEL_FXOSC_CLK,SCU_CLKOUT_SEL_LPO_CLK,SCU_CLKOUT_SEL_CORE_CLK,SCU_CLKOUT_SEL_TRNG_CLK' ',') this.data.clkOut.ref}},
            .divider = {{this.data.clkOut.div}}
        },
};

{{#isTruthy (or this.data.cmu0.enable this.data.cmu1.enable)}}
const cmu_config_t {{this.name}}CmuConfig = {
    .fircClockMonitor={
        .enable = {{this.data.cmu0.enable}},                 
        .resetEnable = {{this.data.cmu0.reset}},           
        .refClock = CMU_REF_SIRC_CLOCK,  
        .compareHigh = ({{this.data.cmu0.compareHigh}} * 128 / 2),        
        .compareLow = ({{this.data.cmu0.compareLow}} * 128 / 2),  
    },
    .fxoscClockMonitor={
        .enable = {{this.data.cmu1.enable}},                 
        .resetEnable = {{this.data.cmu1.reset}},           
        .refClock = CMU_REF_SIRC_CLOCK,              
        .compareHigh = ({{this.data.cmu1.compareHigh}} * 128 / 2),        
        .compareLow = ({{this.data.cmu1.compareLow}} * 128 / 2),         
    },
};
{{/isTruthy}}

/*! @brief User Configuration structure {{this.name}}ClockManager */
clock_manager_user_config_t {{this.name}}ClockManager = {
    .scuConfigPtr = &{{this.name}}ScuConfig,
    .cmuConfigPtr = {{#isTruthy (or this.data.cmu0.enable this.data.cmu1.enable)}}&{{this.name}}CmuConfig{{else}}NULL{{/isTruthy}},
    .ipcConfig =
        {
            .peripheralClocks = {{this.name}}PeripheralClockConfig,
            .count = {{get 'length' (filter this.data.periClocks true property="userCtrl")}},
        },
};
{{/each}}

/*! @brief Array of pointers to User configuration structures */
clock_manager_user_config_t const *g_clockManConfigsArr[] = {
{{#each this.data.clockArray}}
    &{{this.name}}ClockManager,
{{/each}}
};

/*! @brief Array of pointers to User defined Callbacks configuration structures */
/* The tool do not support generate Callbacks configuration. It's always empty. */
clock_manager_callback_user_config_t *g_clockManCallbacksArr[] = {(void *)0};
