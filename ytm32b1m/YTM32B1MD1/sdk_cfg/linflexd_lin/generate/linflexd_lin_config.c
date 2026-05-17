{{ytCopyright "linflexd_lin_config.c"}}

#include <stddef.h>
#include "linflexd_lin_config.h"


{{#each this.data}}
/*{{this.name}}*/
linflexd_state_t {{this.name}}_State;
{{#if this.filter.length}}
static linflexd_id_filter_config_t {{this.name}}Filter[{{this.filter.length}}]={
    {{#each this.filter}}
    {
        .filterEnable={{this.filterEnable}},
        .maskEnable={{this.maskEnable}},
        .dataFieldLength={{this.dataFieldLength}},
        .direction={{this.direction}},
        .checksumType={{this.checksumType}},
        .id={{this.id}},
        .data={ {{#each this.data}}{{this}},{{/each}} },
    },
    {{/each}}
};
{{/if}}
linflexd_user_config_t {{this.name}} = {
    .baudRate={{this.baudrate}}U,
    .nodeFunction={{this.nodeFunction}},
    .breakLength={{this.breakLength}},
    .autobaudEnable={{this.autobaudEnable}},
    .timeoutEnable={{this.timeoutEnable}},
    .responseTimeoutValue={{this.responseTimeoutValue}},
    .headerTimeoutValue={{this.headerTimeoutValue}},
    .filterCount={{this.filter.length}},
    .slaveFilterCfgPtr={{#if this.filter.length}}{{this.name}}Filter{{else}}NULL{{/if}},
};
{{/each}}

