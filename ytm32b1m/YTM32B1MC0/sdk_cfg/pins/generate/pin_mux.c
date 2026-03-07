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
    {{#inArray (jsonParse '["YTM32B1LE1","YTM32Z1MD0","YTM32B1MC0", "YTM32Z1ME0", "YTM32B1MD2"]') (truncate (getProjectInfo 'partNumber') 10)}}
    {{else}}
        .rateSelect = {{this.rateSelect}},
    {{/inArray}}
    {{#inArray (jsonParse '["YTM32Z1MD0", "YTM32Z1ME0"]') (truncate (getProjectInfo 'partNumber') 10)}}
    {{else}}
        .passiveFilter = {{this.passiveFilter}},
        .driveSelect = {{this.strength}},
    {{/inArray}}
        .mux = {{this.alt}},
    {{#inArray (jsonParse '["YTM32B1HA0"]') (truncate (getProjectInfo 'partNumber') 10)}}
        .pinLock = {{#isTruthy this.pinLock}}true{{else}}false{{/isTruthy}},
    {{else}}
    {{/inArray}}
        .intConfig = {{this.intCfg}},
        .clearIntFlag = {{this.intFlag}},
        .digitalFilter = {{this.filter}},
    {{#inArray (jsonParse '["YTM32B1LE1","YTM32B1LE0","YTM32Z1LS0"]') (truncate (getProjectInfo 'partNumber') 10)}}
        .filterConfig ={
            .width = {{this.digitalWidth}}
        },
    {{else}}
    {{/inArray}}
        .gpioBase = {{this.gpioBase}},
        .direction = {{#eq this.direction ''}}GPIO_INPUT_DIRECTION{{else}}{{this.direction}}{{/eq}},
        .initValue = {{this.init}},
    },
    {{/each}}
};


{{/if}}
{{/each}}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/

