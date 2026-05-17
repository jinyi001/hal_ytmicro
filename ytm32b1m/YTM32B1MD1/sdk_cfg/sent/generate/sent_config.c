{{ytCopyright "sent_config.c"}}


#include "sent_config.h"

/* SENT channel configuration */
const sent_channel_config_t sentChannelConfig[{{this.data.numOfConfigs}}] = 
{
{{#each this.data.channelConfig}}
    /* {{this.name}} */
    {
        .channelId = {{this.channelId}},
        .tickDuration = {{this.tickDuration}},
        .inputFilter = {{this.inputFilter}},
        .diagConfig =
        {
            .diagEvents = (
                            SENT_EV_CAL_RESYNC |
                            SENT_EV_CAL_20_25 |
                            SENT_EV_SMSG_OFLW |
                            SENT_EV_FMSG_OFLW |
                            SENT_EV_PP_DIAG_ERR |
                            SENT_EV_CAL_LEN_ERR |
                            SENT_EV_CAL_DIAG_ERR |
                            SENT_EV_NIB_VAL_ERR |
                            SENT_EV_SMSG_CRC_ERR |
                            SENT_EV_FMSG_CRC_ERR |
                            SENT_EV_NUM_EDGES_ERR |
                            SENT_EV_NONE
                          ),
            .idleCount = {{this.idleCount}},
            .calibVar = {{this.calibVar}},
            .diagPulse = {{this.diagPulse}},
            .pausePulse = {{this.pausePulse}},
            .successiveCal = {{this.successiveCal}},
        },
        .fastMsgConfig =
        {
            .numberOfNibbles = {{this.fastMsgNibbles}},
            .dmaEnable = {{this.fastMsgDmaEnable}},
            .crcIncludeStatus = {{this.fastMsgCrcIncludeStatus}},
            .disableCrcCheck = {{this.fastMsgCrcCheckDisable}},
            .crcType = {{this.fastMsgCrcType}},
        },
        .slowMsgConfig =
        {
            .dmaEnable = {{this.slowMsgDmaEnable}},
            .crcType = {{this.slowMsgCrcType}},
        }
    },
{{/each}}
};

/* SENT user configuration */
{{#isTruthy this.data.readonly}}const {{/isTruthy}}sent_driver_user_config_t sentUserConfig = {
{{#if (or (eq this.data.fastMsgDmaPtr "") (eq this.data.fastMsgDmaPtr "NULL"))}}
    .fastMsgDmaPtr = NULL,
{{else}}
    .fastMsgDmaPtr = &{{this.data.fastMsgDmaPtr}},
{{/if}}
{{#if (or (eq this.data.slowMsgDmaPtr "") (eq this.data.slowMsgDmaPtr "NULL"))}}
    .slowMsgDmaPtr = NULL,
{{else}}
    .slowMsgDmaPtr = &{{this.data.slowMsgDmaPtr}},
{{/if}}
    .fastDmaChannel = {{this.data.fastDmaChannel}},
    .slowDmaChannel = {{this.data.slowDmaChannel}},
    .callbackFunc = 
    {
{{#if (or (eq this.data.callbackFunction "") (eq this.data.callbackFunction "NULL"))}}
        .function = NULL,
{{else}}
        .function = {{this.data.callbackFunction}},
{{/if}}
{{#if (or (eq this.data.callbackParam "") (eq this.data.callbackParam "NULL"))}}
        .param = NULL,
{{else}}
        .param = &{{this.data.callbackParam}},
{{/if}}
    },
    .fastDmaFIFOEnable = {{this.data.fastDmaFifoEnable}},
    .fastDmaFIFOSize = {{this.data.fastDmaFifoSize}},
    .channelConfig = (const sent_channel_config_t *)sentChannelConfig,
    .numOfConfigs = {{this.data.numOfConfigs}},
};

sent_state_t sentState;

