{{ytCopyright "can_config.c"}}


#include "can_config.h"


{{#each this.data}}
/*{{this.name}}*/

flexcan_state_t {{this.name}}_State;

{{#isTruthy this.readonly}}const{{/isTruthy}} flexcan_user_config_t {{this.name}} = {
    .max_num_mb = {{this.max_num_mb}}UL,
    .num_id_filters = {{this.num_id_filters}},
    .is_rx_fifo_needed = {{this.is_rx_fifo_needed}},
    .flexcanMode = {{this.flexcanMode}},
    .payload = {{this.payload}},
    .fd_enable = {{this.fd_enable}},
    .pe_clock = {{this.pe_clock}},
    .transfer_type = {{this.transfer_type}},
    .bitrate = {
        .propSeg = {{subtract this.bitrate.propSeg 1}},    /*!< Propagation segment*/
        .phaseSeg1 = {{subtract this.bitrate.phaseSeg1 1}},  /*!< Phase segment 1*/
        .phaseSeg2 = {{subtract this.bitrate.phaseSeg2 1}},  /*!< Phase segment 2*/
        .preDivider = {{subtract this.bitrate.preDivider 1}}, /*!< Clock prescaler division factor*/
        .rJumpwidth = {{subtract this.bitrate.rJumpwidth 1}}, /*!< Resync jump width*/
    },
    .bitrate_cbt = {
        .propSeg = {{this.bitrate_cbt.propSeg}},    /*!< Propagation segment*/
        .phaseSeg1 = {{subtract this.bitrate_cbt.phaseSeg1 1}},  /*!< Phase segment 1*/
        .phaseSeg2 = {{subtract this.bitrate_cbt.phaseSeg2 1}},  /*!< Phase segment 2*/
        .preDivider = {{subtract this.bitrate_cbt.preDivider 1}}, /*!< Clock prescaler division factor*/
        .rJumpwidth = {{subtract this.bitrate_cbt.rJumpwidth 1}}, /*!< Resync jump width*/
    },
    .rxFifoDMAChannel={{this.rxFifoDMAChannel}},
{{#if (getGlobalVar 'meta.FEATURE_CAN_HAS_ENHANCE_FIFO')}}
    .rxFifoDMALastWord={{this.rxFifoDMALastWord}},
    .is_enhance_rx_fifo_needed={{this.is_enhance_rx_fifo_needed}},
    .num_enhance_rx_fifo_filters={{this.num_enhance_rx_fifo_filters}},
    .num_enhance_rx_fifo_extid_filters={{this.num_enhance_rx_fifo_extid_filters}},
    .num_enhance_rx_fifo_min_messages={{this.num_enhance_rx_fifo_min_messages}},
{{/if}}
};
{{/each}}


