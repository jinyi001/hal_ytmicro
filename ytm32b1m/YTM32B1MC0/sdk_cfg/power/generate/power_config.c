{{ytCopyright "power_config.c"}}

#include <stddef.h>
#include "power_config.h"

{{#each this.data.callback}}
extern status_t {{this.callbackFunction}}(power_manager_notify_struct_t *notify,power_manager_callback_data_t *dataPtr);
{{/each}}



{{#each this.data.user}}
/* {{this.name}} */
power_manager_user_config_t {{this.name}}={
    .powerMode={{this.powerMode}},
    .sleepOnExitValue={{this.sleepOnExitValue}},
};
{{/each}}


{{#each this.data.callback}}
/* {{this.name}} */
power_manager_callback_user_config_t {{this.name}}Callback={
    .callbackFunction={{this.callbackFunction}},
    .callbackType={{this.callbackType}},
    .callbackData={{this.callbackData}},
};
{{/each}}



power_manager_user_config_t* powerConfigsArr[{{this.data.user.length}}]={
{{#each this.data.user}}
    &{{this.name}},
{{/each}}
};

{{#if this.data.callback.length}}
power_manager_callback_user_config_t* powerStaticCallbacksConfigsArr[{{this.data.callback.length}}]={
{{#each this.data.callback}}
    &{{this.name}}Callback,
{{/each}}
};
{{/if}}

