{{ytCopyright "pin_mux.c"}}


#include "pin_mux.h"

{{#each this.data.cfgs}}
{{#if (gt this.pins.length 0)}}
/*! @brief User configuration cfg {{this.name}} array */
const pin_settings_config_t g_pin_mux_{{this.name}}[NUM_OF_CONFIGURED_PINS{{@index}}] = {
    {{#each this.pins}}
    /* {{this.label}}-{{this.num}}-{{this.feature}}-{{this.userLabel}} */
    {
        .base = {{this.base}},
        .pinPortIdx = {{itemAt (split this.label '_') 1}}U,
        .pullConfig = {{#if (and this.pullEnable this.pullSelect)}}{{this.pullSelect}}{{else}}PCTRL_INTERNAL_PULL_NOT_ENABLED{{/if}},
        {{! /* The fellowing chip does not support rateSelect */}}
    {{#inArray (jsonParse '["YTM32B1LE1", "YTM32Z1LS1", "YTM32Z1MD0","YTM32B1MC0", "YTM32Z1ME0", "YTM32B1MD2"]') (truncate (getProjectInfo 'partNumber') 10)}}
    {{else}}
        .rateSelect = {{this.rateSelect}},
    {{/inArray}}
    {{#inArray (jsonParse '["YTM32Z1MD0", "YTM32Z1ME0"]') (truncate (getProjectInfo 'partNumber') 10)}}
    {{else}}
        .passiveFilter = {{this.passiveFilter}},
        .driveSelect = {{this.strength}},
    {{/inArray}}
        .mux = {{this.alt}},
    {{#inArray (jsonParse '["YTM32B1HA0", "YTM32B1LE1", "YTM32B1MC0", "YTM32B1MD2", "YTM32B1ME1", "YTM32Z1LS1", "YTM32Z1MD0", "YTM32Z1ME0"]') (truncate (getProjectInfo 'partNumber') 10)}}
        .pinLock = {{#isTruthy this.pinLock}}true{{else}}false{{/isTruthy}},
    {{else}}
    {{/inArray}}
        .intConfig = {{this.intCfg}},
        .clearIntFlag = {{this.intFlag}},
        .digitalFilter = {{this.filter}},
    {{#inArray (jsonParse '["YTM32Z1ME0", "YTM32Z1MD0"]') (truncate (getProjectInfo 'partNumber') 10)}}
    {{else}}
        .filterConfig ={
            .width = {{#isTruthy this.digitalWidth}}{{this.digitalWidth}}U{{else}}0U{{/isTruthy}},
        },
    {{/inArray}}
        .gpioBase = {{this.gpioBase}},
        .direction = {{#eq this.direction ''}}GPIO_INPUT_DIRECTION{{else}}{{this.direction}}{{/eq}},
        .initValue = {{this.init}},
    },
    {{/each}}
    {{#inArray (jsonParse '["YTM32Z1LS1"]') (truncate (getProjectInfo 'partNumber') 10)}}
    /* PTA_13-LIN_SLEEP_EN */
    {
        .base = PCTRLA,
        .pinPortIdx = 13U,
        .pullConfig = PCTRL_INTERNAL_PULL_NOT_ENABLED,
        .passiveFilter = false,
        .driveSelect = PCTRL_LOW_DRIVE_STRENGTH,
        .mux = PCTRL_MUX_AS_GPIO,
        .pinLock = false,
        .intConfig = PCTRL_DMA_INT_DISABLED,
        .clearIntFlag = true,
        .digitalFilter = false,
        .filterConfig = {
            .width = 8U,
        },
        .gpioBase = GPIOA,
        .direction = GPIO_OUTPUT_DIRECTION,
        .initValue = 1,
    },
    /* PTC_8-LIN_UART1_RX */
    {
        .base = PCTRLC,
        .pinPortIdx = 8U,
        .pullConfig = PCTRL_INTERNAL_PULL_NOT_ENABLED,
        .passiveFilter = false,
        .driveSelect = PCTRL_LOW_DRIVE_STRENGTH,
        .mux = PCTRL_MUX_ALT2,
        .pinLock = false,
        .intConfig = PCTRL_DMA_INT_DISABLED,
        .clearIntFlag = false,
        .digitalFilter = false,
        .filterConfig = {
            .width = 8U,
        },
        .gpioBase = GPIOC,
        .direction = GPIO_OUTPUT_DIRECTION,
        .initValue = 1,
    },
    /* PTC_9-LIN_UART1_TX */
    {
        .base = PCTRLC,
        .pinPortIdx = 9U,
        .pullConfig = PCTRL_INTERNAL_PULL_NOT_ENABLED,
        .passiveFilter = false,
        .driveSelect = PCTRL_LOW_DRIVE_STRENGTH,
        .mux = PCTRL_MUX_ALT2,
        .pinLock = false,
        .intConfig = PCTRL_DMA_INT_DISABLED,
        .clearIntFlag = false,
        .digitalFilter = false,
        .filterConfig = {
            .width = 8U,
        },
        .gpioBase = GPIOC,
        .direction = GPIO_OUTPUT_DIRECTION,
        .initValue = 1,
    },
    {{else}}
    {{/inArray}}
};


{{/if}}
{{/each}}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/

